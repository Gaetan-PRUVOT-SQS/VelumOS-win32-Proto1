#!/usr/bin/env python3
"""Image disque démarrable de VelumOS : GPT, Limine en BIOS et en UEFI, sans droits root.

mkimage.py --noyau kernel.elf --sortie disk.img [--initrd DOSSIER] [--limine DOSSIER]
           [--resolution 1024x768] [--cmdline TEXTE] [--esp-mio 64]
mkimage.py --comparer CONSTRUCTION_A CONSTRUCTION_B

--comparer compare deux constructions (dossiers contenant kernel.elf, disk.img et root/) :
empreintes SHA-256 du noyau, de l'initrd relu dans l'image, de l'arbre root/ et de l'image,
premier octet différent s'il y en a, code de sortie 1 en cas de différence (`make repro`).

Partition 1 : ESP FAT (Limine, limine.conf, noyau, initrd au format cpio newc).
Partition 2 : démarrage BIOS (1 Mio, second étage de Limine pour GPT).
Outils externes : sgdisk (gdisk), mtools (mformat, mmd, mcopy) et l'outil `limine`
que `make limine` prépare dans build/limine.

L'image est reproductible : GUID du disque et des partitions, numéro de série FAT et dates
des fichiers sont dérivés de SOURCE_DATE_EPOCH (0 si absent), jamais de l'heure ni du hasard.
limine.conf porte le BLAKE2b du noyau et de l'initrd, et Limine panique s'ils ne concordent pas.
"""

import argparse
import hashlib
import os
import shutil
import stat
import subprocess
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path

SECTEUR = 512
MIO = 1 << 20
RACINE = Path(__file__).resolve().parent.parent
FICHIERS_EFI = ("BOOTX64.EFI", "BOOTIA32.EFI", "BOOTAA64.EFI", "BOOTRISCV64.EFI")
NOM_ENTREE = "VelumOS"
ESPACE_IDENTIFIANTS = uuid.UUID("7c1d2a54-3e0b-5c4f-9a6e-56656c756d4f")
EPOQUE_FAT_MIN = 315532800
EPOQUE_DEFAUT = 0


class Erreur(Exception):
    pass


@dataclass(frozen=True)
class Plan:
    taille: int
    bios_debut: int
    bios_taille: int
    esp_debut: int
    esp_taille: int


def planifier(esp_mio):
    if esp_mio < 8:
        raise Erreur("ESP de 8 Mio au moins")
    bios_debut = MIO
    esp_debut = 2 * MIO
    esp_taille = esp_mio * MIO
    taille = esp_debut + esp_taille + MIO
    return Plan(taille, bios_debut, MIO, esp_debut, esp_taille)


@dataclass(frozen=True)
class Identifiants:
    disque: str
    esp: str
    bios: str
    serie_fat: str


def epoque_source():
    brut = os.environ.get("SOURCE_DATE_EPOCH", str(EPOQUE_DEFAUT))
    if not brut.isascii() or not brut.isdigit():
        raise Erreur(f"SOURCE_DATE_EPOCH invalide : {brut!r}")
    return int(brut)


def identifiants(epoque):
    def derive(role):
        return uuid.uuid5(ESPACE_IDENTIFIANTS, f"{epoque}/{role}")

    serie = derive("fat").int >> 96
    return Identifiants(
        str(derive("disque")).upper(),
        str(derive("esp")).upper(),
        str(derive("bios")).upper(),
        f"{serie:08X}",
    )


def environnement_outils(epoque):
    return {
        **os.environ,
        "SOURCE_DATE_EPOCH": str(max(epoque, EPOQUE_FAT_MIN)),
        "TZ": "UTC",
        "LC_ALL": "C",
    }


def empreinte_blake2b(donnees):
    return hashlib.blake2b(donnees).hexdigest()


def lancer(commande, env=None):
    try:
        resultat = subprocess.run(commande, capture_output=True, check=False, env=env)
    except FileNotFoundError as erreur:
        raise Erreur(f"outil absent : {commande[0]}") from erreur
    if resultat.returncode != 0:
        detail = resultat.stderr.decode(errors="replace").strip()[-400:]
        raise Erreur(f"{Path(commande[0]).name} a échoué : {detail}")
    return resultat.stdout


def entree_cpio(ino, mode, taille, nom):
    nom_nul = nom.encode() + b"\0"
    nlink = 2 if stat.S_ISDIR(mode) else 1
    mtime = epoque_source()
    champs = (ino, mode, 0, 0, nlink, mtime, taille, 0, 0, 0, 0, len(nom_nul), 0)
    brut = b"070701" + "".join(f"{c:08X}" for c in champs).encode() + nom_nul
    return brut + b"\0" * (-len(brut) % 4)


def cpio_newc(dossier):
    sortie = bytearray(entree_cpio(1, stat.S_IFDIR | 0o755, 0, "."))
    chemins = sorted(dossier.rglob("*"), key=lambda p: p.relative_to(dossier).as_posix())
    for numero, chemin in enumerate(chemins, start=2):
        nom = chemin.relative_to(dossier).as_posix()
        if chemin.is_symlink():
            raise Erreur(f"lien symbolique refusé dans l'initrd : {nom}")
        if chemin.is_dir():
            sortie += entree_cpio(numero, stat.S_IFDIR | 0o755, 0, nom)
            continue
        donnees = chemin.read_bytes()
        droits = 0o755 if os.access(chemin, os.X_OK) else 0o644
        sortie += entree_cpio(numero, stat.S_IFREG | droits, len(donnees), nom)
        sortie += donnees + b"\0" * (-len(donnees) % 4)
    sortie += entree_cpio(0, 0, 0, "TRAILER!!!")
    return bytes(sortie)


def avec_empreinte(chemin, empreinte):
    return chemin if empreinte is None else f"{chemin}#{empreinte}"


def config_limine(resolution, cmdline, avec_initrd, hash_noyau=None, hash_initrd=None):
    lignes = [
        "timeout: 0",
        "hash_mismatch_panic: yes",
        "",
        f"/{NOM_ENTREE}",
        "    protocol: limine",
        f"    path: {avec_empreinte('boot():/boot/kernel.elf', hash_noyau)}",
    ]
    if cmdline:
        lignes.append(f"    cmdline: {cmdline}")
    if avec_initrd:
        module = avec_empreinte("boot():/boot/initrd.cpio", hash_initrd)
        lignes.append(f"    module_path: {module}")
    lignes.append(f"    resolution: {resolution}x32")
    return "\n".join(lignes) + "\n"


def commande_gpt(image, plan, ident):
    esp_fin = (plan.esp_debut + plan.esp_taille) // SECTEUR - 1
    bios_fin = (plan.bios_debut + plan.bios_taille) // SECTEUR - 1
    return [
        "sgdisk", "-o", "-U", ident.disque,
        "-n", f"1:{plan.esp_debut // SECTEUR}:{esp_fin}", "-t", "1:EF00", "-c", "1:ESP",
        "-u", f"1:{ident.esp}",
        "-n", f"2:{plan.bios_debut // SECTEUR}:{bios_fin}", "-t", "2:EF02", "-c", "2:bios",
        "-u", f"2:{ident.bios}",
        str(image),
    ]  # fmt: skip


def remplir_esp(image, plan, fichiers, ident, env):
    cible = f"{image}@@{plan.esp_debut}"
    lancer(["mformat", "-i", cible, "-T", str(plan.esp_taille // SECTEUR),
            "-h", "64", "-s", "32", "-v", "ESP", "-N", ident.serie_fat, "::"], env)  # fmt: skip
    lancer(["mmd", "-i", cible, "::/EFI", "::/EFI/BOOT", "::/boot", "::/boot/limine"], env)
    for destination, source in sorted(fichiers.items()):
        lancer(["mcopy", "-i", cible, str(source), "::" + destination], env)


def relire_esp(image, plan, destination, env):
    cible = f"{image}@@{plan.esp_debut}"
    return lancer(["mcopy", "-i", cible, "::" + destination, "-"], env)


def fichiers_esp(travail, args, limine, initrd):
    noyau = travail / "kernel.elf"
    shutil.copyfile(args.noyau, noyau)
    hash_noyau = empreinte_blake2b(noyau.read_bytes())
    hash_initrd = None if initrd is None else empreinte_blake2b(initrd)
    conf = travail / "limine.conf"
    conf.write_text(
        config_limine(args.resolution, args.cmdline, initrd is not None, hash_noyau, hash_initrd)
    )
    fichiers = {
        "/boot/kernel.elf": noyau,
        "/boot/limine/limine.conf": conf,
        "/boot/limine/limine-bios.sys": limine / "limine-bios.sys",
    }
    for nom in FICHIERS_EFI:
        fichiers[f"/EFI/BOOT/{nom}"] = limine / nom
    if initrd is not None:
        archive = travail / "initrd.cpio"
        archive.write_bytes(initrd)
        fichiers["/boot/initrd.cpio"] = archive
    return fichiers, {"/boot/kernel.elf": hash_noyau, "/boot/initrd.cpio": hash_initrd}


def verifier_empreintes(image, plan, attendues, env):
    for destination, attendue in attendues.items():
        if attendue is None:
            continue
        relue = empreinte_blake2b(relire_esp(image, plan, destination, env))
        if relue != attendue:
            raise Erreur(f"{destination} relu dans l'image : empreinte différente de limine.conf")


def verifier_entrees(args, limine):
    if not args.noyau.is_file():
        raise Erreur(f"noyau introuvable : {args.noyau}")
    if args.initrd is not None and not args.initrd.is_dir():
        raise Erreur(f"dossier introuvable : {args.initrd}")
    manquants = [
        n for n in ("limine", "limine-bios.sys", *FICHIERS_EFI) if not (limine / n).is_file()
    ]
    if manquants:
        raise Erreur(
            f"Limine incomplet dans {limine} ({', '.join(manquants)}) : lancer `make limine`"
        )


def fabriquer(args):
    limine = args.limine
    verifier_entrees(args, limine)
    plan = planifier(args.esp_mio)
    epoque = epoque_source()
    ident = identifiants(epoque)
    env = environnement_outils(epoque)
    image = args.sortie
    travail = image.parent / (image.name + ".travail")
    travail.mkdir(parents=True, exist_ok=True)
    initrd = cpio_newc(args.initrd) if args.initrd else None
    image.unlink(missing_ok=True)
    with image.open("wb") as flux:
        flux.truncate(plan.taille)
    lancer(commande_gpt(image, plan, ident), env)
    fichiers, empreintes = fichiers_esp(travail, args, limine, initrd)
    remplir_esp(image, plan, fichiers, ident, env)
    verifier_empreintes(image, plan, empreintes, env)
    lancer([str(limine / "limine"), "bios-install", str(image)], env)
    lancer(["sgdisk", "-v", str(image)])
    shutil.rmtree(travail)
    return plan


ELEMENTS_COMPARES = ("kernel.elf", "initrd.cpio", "rootfs", "disk.img")
LIGNES_DIFFERENCES_MAX = 20


def manifeste_arbre(racine):
    lignes = []
    for chemin in sorted(racine.rglob("*"), key=lambda c: c.relative_to(racine).as_posix()):
        nom = chemin.relative_to(racine).as_posix()
        if chemin.is_symlink():
            lignes.append(f"lien {os.readlink(chemin)} {nom}")
        elif chemin.is_dir():
            lignes.append(f"dossier {nom}")
        else:
            droits = "x" if os.access(chemin, os.X_OK) else "-"
            lignes.append(f"fichier {droits} {empreinte_sha256(chemin.read_bytes())} {nom}")
    return ("\n".join(lignes) + "\n").encode()


def empreinte_sha256(donnees):
    return hashlib.sha256(donnees).hexdigest()


def extraire_initrd(image):
    cible = f"{image}@@{planifier(8).esp_debut}"
    return lancer(
        ["mcopy", "-i", cible, "::/boot/initrd.cpio", "-"],
        environnement_outils(epoque_source()),
    )


def elements_construction(dossier, extraire=None):
    extraire = extraire or extraire_initrd
    dossier = Path(dossier)
    for nom, chemin in (("kernel.elf", dossier / "kernel.elf"), ("disk.img", dossier / "disk.img")):
        if not chemin.is_file():
            raise Erreur(f"{nom} absent de la construction : {chemin}")
    if not (dossier / "root").is_dir():
        raise Erreur(f"root/ absent de la construction : {dossier}")
    return {
        "kernel.elf": (dossier / "kernel.elf").read_bytes(),
        "initrd.cpio": extraire(dossier / "disk.img"),
        "rootfs": manifeste_arbre(dossier / "root"),
        "disk.img": (dossier / "disk.img").read_bytes(),
    }


def premier_octet_different(a, b):
    commun = min(len(a), len(b))
    pas = 1 << 16
    for debut in range(0, commun, pas):
        fin = min(debut + pas, commun)
        if a[debut:fin] != b[debut:fin]:
            return next(i for i in range(debut, fin) if a[i] != b[i])
    return None if len(a) == len(b) else commun


def region_image(decalage, taille):
    esp_debut = planifier(8).esp_debut
    if decalage < SECTEUR:
        return "MBR de protection et code d'amorçage BIOS"
    if decalage < 2 * SECTEUR:
        return "en-tête GPT (GUID du disque)"
    if decalage < MIO:
        return "entrées GPT (GUID des partitions)"
    if decalage < esp_debut:
        return "partition de démarrage BIOS"
    if decalage < esp_debut + SECTEUR:
        return "secteur de démarrage FAT (numéro de série)"
    if decalage >= taille - 33 * SECTEUR:
        return "copie de secours de la GPT"
    return "système de fichiers de l'ESP (FAT, dossiers, dates ou contenu)"


def lignes_difference(nom, a, b):
    if nom == "rootfs":
        seules_a = sorted(set(a.decode().splitlines()) - set(b.decode().splitlines()))
        seules_b = sorted(set(b.decode().splitlines()) - set(a.decode().splitlines()))
        lignes = [f"    A seulement : {ligne}" for ligne in seules_a[:LIGNES_DIFFERENCES_MAX]]
        return lignes + [
            f"    B seulement : {ligne}" for ligne in seules_b[:LIGNES_DIFFERENCES_MAX]
        ]
    rang = premier_octet_different(a, b)
    if rang is None:
        return []
    if rang >= min(len(a), len(b)):
        return [f"    tailles différentes : A={len(a)} octets, B={len(b)} octets"]
    lignes = [
        f"    premier octet différent : {rang} (0x{rang:x}), A=0x{a[rang]:02x} B=0x{b[rang]:02x}"
    ]
    if nom == "disk.img":
        lignes.append(f"    région : {region_image(rang, len(a))}")
    return lignes


def comparer_elements(elements_a, elements_b):
    rapport, identiques = [], True
    for nom in ELEMENTS_COMPARES:
        a, b = elements_a[nom], elements_b[nom]
        rapport.append(f"{nom:12} A {empreinte_sha256(a)}")
        rapport.append(f"{nom:12} B {empreinte_sha256(b)}")
        if a == b:
            rapport.append(f"{nom:12} identique")
            continue
        identiques = False
        rapport.append(f"{nom:12} DIFFERENT")
        rapport.extend(lignes_difference(nom, a, b))
    return identiques, rapport


def comparer_constructions(dossier_a, dossier_b):
    identiques, rapport = comparer_elements(
        elements_construction(dossier_a), elements_construction(dossier_b)
    )
    verdict = "reproductible : oui" if identiques else "reproductible : NON"
    return identiques, "\n".join([*rapport, verdict])


def analyser(argv):
    parseur = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parseur.add_argument("--noyau", type=Path)
    parseur.add_argument("--sortie", type=Path)
    parseur.add_argument("--comparer", type=Path, nargs=2, metavar=("A", "B"))
    parseur.add_argument("--initrd", type=Path)
    defaut = Path(os.environ.get("LIMINE_DIR", RACINE / "build" / "limine"))
    parseur.add_argument("--limine", type=Path, default=defaut)
    parseur.add_argument("--resolution", default="1024x768")
    parseur.add_argument("--cmdline", default="")
    parseur.add_argument("--esp-mio", type=int, default=64)
    args = parseur.parse_args(argv)
    if args.comparer is None and (args.noyau is None or args.sortie is None):
        parseur.error("--noyau et --sortie sont requis (ou --comparer A B)")
    return args


def main(argv):
    args = analyser(argv)
    try:
        if args.comparer is not None:
            identiques, rapport = comparer_constructions(*args.comparer)
            print(rapport)
            return 0 if identiques else 1
        plan = fabriquer(args)
    except Erreur as erreur:
        print(f"mkimage : {erreur}", file=sys.stderr)
        return 1
    taille, esp = plan.taille // MIO, plan.esp_taille // MIO
    print(f"{args.sortie} : {taille} Mio, ESP {esp} Mio, Limine BIOS et UEFI")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
