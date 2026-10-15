import json
import os
import re
import shutil
import struct
import subprocess
import sys
import threading
from pathlib import Path

NAME = "virtio-blk : partitions étendues MBR, chaîne des EBR saine, en boucle et hors limites"
CMDLINE = "selftest exit"

SECTEUR = 512
TOTAL = 16384
TYPE_LINUX = 0x83
TYPE_ETENDUE_LBA = 0x0F
TYPE_ETENDUE_CHS = 0x05
PREMIERE_LOGIQUE = 5
E_PROTO = 71
PRIMAIRE = (2048, 2048)
ETENDUE = (4096, 12288)
ETENDUE_TROP_GRANDE = (4096, 12289)
CHAINE_SAINE = ((4096, 63, 1985, 2048), (6144, 63, 3000, 6144), (10240, 2048, 4096, None))
CHAINE_EN_BOUCLE = ((4096, 63, 1985, 2048), (6144, 63, 3000, 0))
CHAINE_HORS_DISQUE = ((4096, 63, 1985, 12388),)
PRIMAIRE_SUR_ETENDUE = (2048, 4096)
CHAINE_DANS_PRIMAIRE = ((4096, 4096, 1024, None),)
DISQUES = (
    ("vda", "saine", PRIMAIRE, ETENDUE, CHAINE_SAINE, 3),
    ("vdb", "boucle", PRIMAIRE, ETENDUE, CHAINE_EN_BOUCLE, 2),
    ("vdc", "hors_disque", PRIMAIRE, ETENDUE, CHAINE_HORS_DISQUE, 1),
    ("vdd", "etendue_trop_grande", PRIMAIRE, ETENDUE_TROP_GRANDE, CHAINE_SAINE, 0),
    ("vde", "sur_primaire", PRIMAIRE_SUR_ETENDUE, ETENDUE, CHAINE_DANS_PRIMAIRE, 0),
)
AVERTISSEMENTS = (
    rf"W block: vdb: chaîne étendue arrêtée au maillon 2 \(-{E_PROTO}\)",
    rf"W block: vdc: chaîne étendue arrêtée au maillon 1 \(-{E_PROTO}\)",
    r"W block: vdd: partition étendue hors limites ou en trop, non suivie",
    r"W block: vde: partition étendue sur une primaire, non suivie",
)
RACINE = Path(__file__).resolve().parent.parent.parent
_VERROU = threading.Lock()
_FAITES = []


def _dossier():
    argv = sys.argv
    if "--out" in argv and argv.index("--out") + 1 < len(argv):
        return Path(argv[argv.index("--out") + 1]).resolve() / "a11_etendue_donnees"
    return RACINE / "build" / "a11" / "qemu" / "a11_etendue_donnees"


def _chemin(suffixe):
    return _dossier() / f"a11_etendue_{suffixe}.img"


def _table(entrees):
    secteur = bytearray(SECTEUR)
    for rang, (type_part, debut, nombre) in enumerate(entrees):
        base = 446 + 16 * rang
        secteur[base + 4] = type_part
        struct.pack_into("<II", secteur, base + 8, debut, nombre)
    secteur[510:512] = b"\x55\xaa"
    return secteur


def _taille_du_lien(chaine, rang):
    if rang + 1 < len(chaine):
        _, decalage, taille, _ = chaine[rang + 1]
        return decalage + taille
    return 1


def image(primaire, etendue, chaine):
    donnees = bytearray(TOTAL * SECTEUR)
    donnees[0:SECTEUR] = _table([(TYPE_LINUX, *primaire), (TYPE_ETENDUE_LBA, *etendue)])
    for rang, (lba, decalage, taille, lien) in enumerate(chaine):
        entrees = [(TYPE_LINUX, decalage, taille)]
        if lien is not None:
            entrees.append((TYPE_ETENDUE_CHS, lien, _taille_du_lien(chaine, rang)))
        donnees[lba * SECTEUR : (lba + 1) * SECTEUR] = _table(entrees)
    return donnees


def attendu():
    partitions = {}
    for disque, _, primaire, _, chaine, gardees in DISQUES:
        partitions[f"{disque}1"] = primaire
        for rang, (lba, decalage, taille, _) in enumerate(chaine[:gardees]):
            partitions[f"{disque}{PREMIERE_LOGIQUE + rang}"] = (lba + decalage, taille)
    return partitions


def _ecrire(chemin, donnees):
    partiel = chemin.with_suffix(".partiel")
    partiel.write_bytes(donnees)
    os.replace(partiel, chemin)


class _Disques:
    def __iter__(self):
        arguments = []
        with _VERROU:
            _dossier().mkdir(parents=True, exist_ok=True)
            for rang, (_, suffixe, primaire, etendue, chaine, _) in enumerate(DISQUES):
                if not _FAITES:
                    _ecrire(_chemin(suffixe), image(primaire, etendue, chaine))
                arguments += [
                    "-drive",
                    f"file={_chemin(suffixe)},if=none,id=e{rang},format=raw,readonly=on",
                    "-device",
                    f"virtio-blk-pci,drive=e{rang},disable-legacy=on,addr={0x10 + rang:#x}",
                ]
            _FAITES.append(_dossier())
        return iter(arguments)


QEMU_EXTRA = _Disques()


def partitions_vues(journal):
    motif = r"block: (vd[a-e]\d+) début (\d+), (\d+) secteurs \(mbr\)"
    return {nom: (int(debut), int(taille)) for nom, debut, taille in re.findall(motif, journal)}


def logiques_selon_sfdisk(chemin):
    outil = shutil.which("sfdisk") or shutil.which("sfdisk", path="/usr/sbin:/sbin")
    if not outil:
        return None
    resultat = subprocess.run(
        [outil, "--json", str(chemin)], capture_output=True, text=True, check=False
    )
    assert resultat.returncode == 0, f"sfdisk : {resultat.stdout}{resultat.stderr}"
    logiques = {}
    for partition in json.loads(resultat.stdout)["partitiontable"]["partitions"]:
        numero = int(re.search(r"(\d+)$", partition["node"]).group(1))
        if numero >= PREMIERE_LOGIQUE:
            logiques[numero] = (partition["start"], partition["size"])
    return logiques


def comparer_a_sfdisk(vues):
    selon_outil = logiques_selon_sfdisk(_chemin("saine"))
    if selon_outil is None:
        print("a11_etendue : sfdisk absent, comparaison avec les seules valeurs du scénario")
        return
    selon_noyau = {
        int(nom[3:]): valeurs
        for nom, valeurs in vues.items()
        if nom.startswith("vda") and int(nom[3:]) >= PREMIERE_LOGIQUE
    }
    assert selon_noyau == selon_outil, f"noyau {selon_noyau}, sfdisk {selon_outil}"


def run(vm):
    vm.attendre(r"block: vde virtio-blk, 16384 secteurs de 512 octets, lecture seule", delai=60)
    vm.attendre(r"\[TEST\] block \.\.\. OK", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=120)
    journal = vm.serie()
    assert "PANIC" not in journal, f"panique\n{journal[-2000:]}"
    vues = partitions_vues(journal)
    assert vues == attendu(), f"partitions vues {vues}, attendues {attendu()}"
    for motif in AVERTISSEMENTS:
        assert re.search(motif, journal), f"{motif!r} absent du journal\n{journal[-3000:]}"
    assert "block: vda: " not in journal, "la chaîne saine ne devait rien signaler"
    comparer_a_sfdisk(vues)
