import re
import shutil
import subprocess
import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
sys.path.insert(0, str(RACINE / "tests" / "qemu"))
import mkimage  # noqa: E402
from a12_fat32 import anomalies_fsck, motif  # noqa: E402
from vtest import VM  # noqa: E402

NAME = "FAT32 /data en partition GPT : écriture, redémarrage, relecture, fsck.vfat"
CMDLINE = "selftest exit donnees=ecrire"
FIRMWARES = ("bios",)

ESP_MIO = 8
DATA_MIO = 34
LONGUEUR_MOTIF = 70000
LONGUEUR_ETAT = 3000
REGLAGES = "theme=sombre\ntransparence=reduite\n"
IMAGE = []


def _dossier():
    argv = sys.argv
    if "--out" in argv and argv.index("--out") + 1 < len(argv):
        return Path(argv[argv.index("--out") + 1]).resolve() / "a12_persistance"
    return RACINE / "build" / "a12" / "qemu" / "a12_persistance"


def _outil(nom):
    chemin = shutil.which(nom) or shutil.which(nom, path="/usr/sbin:/sbin")
    assert chemin, f"{nom} introuvable : installer gdisk, mtools et dosfstools"
    return chemin


def _lancer(*commande):
    resultat = subprocess.run(commande, capture_output=True, check=False)
    assert resultat.returncode == 0, (
        f"{Path(commande[0]).name} : {resultat.stdout.decode(errors='replace')}"
        f"{resultat.stderr.decode(errors='replace')}"
    )
    return resultat.stdout


def etat_attendu():
    return bytes((i * 13 + 5) % 253 for i in range(LONGUEUR_ETAT))


def creer_disque_gpt(dossier):
    dossier.mkdir(parents=True, exist_ok=True)
    plan = mkimage.planifier(ESP_MIO, DATA_MIO)
    ident = mkimage.identifiants(mkimage.epoque_source())
    env = mkimage.environnement_outils(mkimage.epoque_source())
    image = dossier / "donnees_gpt.img"
    image.unlink(missing_ok=True)
    with image.open("wb") as flux:
        flux.truncate(plan.taille)
    mkimage.lancer(mkimage.commande_gpt(image, plan, ident), env)
    mkimage.formater_data(image, plan, ident, env)
    return image, plan


def options_virtio(image):
    return [
        "-drive",
        f"file={image},if=none,id=d1,format=raw",
        "-device",
        "virtio-blk-pci,drive=d1,disable-legacy=on,addr=0x10",
    ]


class _Disque:
    def __iter__(self):
        image, plan = creer_disque_gpt(_dossier())
        IMAGE[:] = [image, plan]
        return iter(options_virtio(image))


QEMU_EXTRA = _Disque()


def cible_data(image, plan):
    return f"{image}@@{plan.data_debut}"


def lire_data(image, plan, chemin):
    return _lancer(_outil("mcopy"), "-n", "-i", cible_data(image, plan), f"::{chemin}", "-")


def extraire_partition(image, plan, sortie):
    with image.open("rb") as source, sortie.open("wb") as cible:
        source.seek(plan.data_debut)
        cible.write(source.read(plan.data_taille))
    return sortie


def verifier_fsck(image, plan):
    partition = extraire_partition(image, plan, image.with_suffix(".data.img"))
    resultat = subprocess.run(
        [_outil("fsck.vfat"), "-n", str(partition)], capture_output=True, text=True, check=False
    )
    sortie = resultat.stdout + resultat.stderr
    autres = anomalies_fsck(sortie)
    assert not autres, f"anomalies de structure FAT32 : {autres}"
    assert resultat.returncode == 0, f"fsck.vfat code {resultat.returncode}\n{sortie}"


def verifier_contenu(image, plan):
    assert lire_data(image, plan, "/velum/ecrit par velum.txt") == motif()
    assert lire_data(image, plan, "/etat/persistance.bin") == etat_attendu()
    assert lire_data(image, plan, "/etat/reglages.conf") == REGLAGES.encode()


def attendre_arret(vm, delai):
    try:
        vm.processus.wait(timeout=delai)
    except subprocess.TimeoutExpired as erreur:
        raise AssertionError("QEMU devait quitter à la fin des autotests") from erreur


def disque_de_relecture(disque, dossier):
    copie = dossier / "demarrage_relecture.img"
    shutil.copyfile(disque, copie)
    cible = f"{copie}@@{2 * mkimage.MIO}"
    conf = _lancer(_outil("mcopy"), "-n", "-i", cible, "::/boot/limine/limine.conf", "-")
    texte = conf.decode()
    assert "donnees=ecrire" in texte, "limine.conf sans le mot donnees=ecrire"
    nouvelle = dossier / "limine_relecture.conf"
    nouvelle.write_text(re.sub(r"donnees=ecrire", "donnees=relire", texte))
    _lancer(_outil("mcopy"), "-o", "-i", cible, str(nouvelle), "::/boot/limine/limine.conf")
    return copie


def demarrer_relecture(vm, disque):
    return VM(
        disque,
        vm.dossier,
        nom="relecture",
        memoire=vm.memoire,
        smp=vm.smp,
        extra=options_virtio(IMAGE[0]),
        reglages=vm.reglages,
    )


def run(vm):
    for outil in ("sgdisk", "mformat", "mcopy", "fsck.vfat"):
        _outil(outil)
    image, plan = IMAGE
    vm.attendre(r"block: vda3 début \d+, \d+ secteurs \(gpt\)", delai=120)
    vm.attendre(r"vfs: /data monté sur vda3", delai=30)
    vm.attendre(r"vfs: persistance écrite", delai=240)
    vm.attendre(r"\[TEST\] vfs \.\.\. OK", delai=240)
    attendre_arret(vm, delai=60)
    verifier_contenu(image, plan)
    verifier_fsck(image, plan)
    relecture = disque_de_relecture(vm.disque, _dossier())
    with demarrer_relecture(vm, relecture) as second:
        second.attendre(r"vfs: /data monté sur vda3", delai=120)
        second.attendre(r"vfs: persistance relue ok", delai=240)
        second.attendre(r"\[TEST\] vfs \.\.\. OK", delai=240)
        attendre_arret(second, delai=60)
    verifier_contenu(image, plan)
    verifier_fsck(image, plan)
