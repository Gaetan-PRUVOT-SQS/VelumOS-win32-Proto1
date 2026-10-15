import os
import struct
import sys
import threading
import uuid
import zlib
from pathlib import Path

NAME = "virtio-blk : GPT et MBR, lecture, écriture sur disque d'essai"
CMDLINE = "selftest exit blktest"

SECTEUR = 512
MAGIQUE = b"VELUMBLKTEST"
TYPE_LINUX = uuid.UUID("0fc63daf-8483-4772-8e79-3d69d8477de4").bytes_le
ENTREES = 128
TAILLE_ENTREE = 128
SECTEURS_TABLE = ENTREES * TAILLE_ENTREE // SECTEUR
RACINE = Path(__file__).resolve().parent.parent.parent
_VERROU = threading.Lock()
_FAITES = []


def _dossier():
    argv = sys.argv
    if "--out" in argv and argv.index("--out") + 1 < len(argv):
        return Path(argv[argv.index("--out") + 1]).resolve() / "a11_donnees"
    return RACINE / "build" / "a11" / "qemu" / "a11_donnees"


def _motif(nombre):
    donnees = bytearray(nombre * SECTEUR)
    donnees[: len(MAGIQUE)] = MAGIQUE
    for s in range(1, nombre):
        base = s * SECTEUR
        donnees[base : base + SECTEUR] = bytes((s * 31 + i) & 0xFF for i in range(SECTEUR))
    return donnees


def _mbr(type_part, debut, nombre):
    secteur = bytearray(SECTEUR)
    secteur[446 + 4] = type_part
    struct.pack_into("<II", secteur, 446 + 8, debut, nombre)
    secteur[510:512] = b"\x55\xaa"
    return secteur


def _entete(mon_lba, autre_lba, lba_table, total, crc_table):
    champs = struct.pack(
        "<8sIIIIQQQQ16sQIII",
        b"EFI PART",
        0x00010000,
        92,
        0,
        0,
        mon_lba,
        autre_lba,
        2 + SECTEURS_TABLE,
        total - 2 - SECTEURS_TABLE,
        uuid.UUID("5c5c5c5c-0000-4000-8000-a11a11a11a11").bytes_le,
        lba_table,
        ENTREES,
        TAILLE_ENTREE,
        crc_table,
    )
    champs = champs[:16] + struct.pack("<I", zlib.crc32(champs)) + champs[20:]
    return champs + bytes(SECTEUR - len(champs))


def image_gpt(total, debut, fin):
    image = bytearray(total * SECTEUR)
    image[0:SECTEUR] = _mbr(0xEE, 1, total - 1)
    table = bytearray(ENTREES * TAILLE_ENTREE)
    unique = uuid.UUID("a11a11a1-1111-4111-8111-111111111111").bytes_le
    table[0:56] = TYPE_LINUX + unique + struct.pack("<QQQ", debut, fin, 0)
    crc = zlib.crc32(table)
    secours = total - 1 - SECTEURS_TABLE
    image[2 * SECTEUR : 2 * SECTEUR + len(table)] = table
    image[secours * SECTEUR : secours * SECTEUR + len(table)] = table
    image[SECTEUR : 2 * SECTEUR] = _entete(1, total - 1, 2, total, crc)
    image[(total - 1) * SECTEUR :] = _entete(total - 1, 1, secours, total, crc)
    image[debut * SECTEUR : (fin + 1) * SECTEUR] = _motif(fin - debut + 1)
    return image


def image_mbr(total, debut, nombre):
    image = bytearray(total * SECTEUR)
    image[0:SECTEUR] = _mbr(0x83, debut, nombre)
    return image


def _ecrire(chemin, donnees):
    partiel = chemin.with_suffix(".partiel")
    partiel.write_bytes(donnees)
    os.replace(partiel, chemin)


class _Disques:
    def __iter__(self):
        dossier = _dossier()
        with _VERROU:
            dossier.mkdir(parents=True, exist_ok=True)
            essai = dossier / "a11_gpt.img"
            lecture = dossier / "a11_mbr_ro.img"
            if not _FAITES:
                _ecrire(essai, image_gpt(16384, 2048, 6143))
                _ecrire(lecture, image_mbr(4096, 63, 1000))
                _FAITES.append(dossier)
        return iter(
            [
                "-drive",
                f"file={essai},if=none,id=d1,format=raw,snapshot=on",
                "-device",
                "virtio-blk-pci,drive=d1,disable-legacy=on,addr=0x10",
                "-drive",
                f"file={lecture},if=none,id=d2,format=raw,readonly=on",
                "-device",
                "virtio-blk-pci,drive=d2,disable-legacy=on,addr=0x11",
            ]
        )


QEMU_EXTRA = _Disques()


def run(vm):
    vm.attendre(r"block: vda virtio-blk, 16384 secteurs de 512 octets\n", delai=60)
    vm.attendre(r"block: vdb virtio-blk, 4096 secteurs de 512 octets, lecture seule")
    vm.attendre(r"block: vda1 début 2048, 4096 secteurs \(gpt\)")
    vm.attendre(r"block: vdb1 début 63, 1000 secteurs \(mbr\)")
    vm.attendre(r"block: autotest écriture vda1 OK", delai=60)
    vm.attendre(r"\[TEST\] block \.\.\. OK", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
    journal = vm.serie()
    for motif in ("GPT principale", "copie GPT", "désactivé", "refusée"):
        assert motif not in journal, f"{motif!r} inattendu\n{journal[-2000:]}"
