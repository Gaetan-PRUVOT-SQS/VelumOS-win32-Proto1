#!/usr/bin/env python3
"""Fabrique le corpus de départ du fuzzing guidé par la couverture.

Les entrées valides viennent des générateurs des lots hôte (d01, d02, d03, d07)
et de petits générateurs locaux (ELF, cpio, FAT, ACPI). Rien n'est écrit hors de
tests/fuzz/corpus et build/fuzz/fixtures."""

import hashlib
import shutil
import struct
import subprocess
import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
FIXTURES = RACINE / "build" / "fuzz" / "fixtures"
CORPUS = RACINE / "tests" / "fuzz" / "corpus"
PLAFOND_TOTAL = 2 * 1024 * 1024
TAILLE_MAX = {"zip": 8192, "inflate": 1536}


def lancer(*commande):
    subprocess.run([str(c) for c in commande], cwd=RACINE, check=True)


def fixtures():
    for sous in ("d01", "d02", "d03", "d07"):
        (FIXTURES / sous).mkdir(parents=True, exist_ok=True)
    lancer("python3", "tests/host/d01/mkfix.py", FIXTURES / "d01")
    lancer("python3", "tests/host/d02/gen_fixtures.py", FIXTURES / "d02")
    lancer("python3", "tests/host/d03/gen_dex.py", "--sortie", FIXTURES / "d03" / "classes.dex")
    dasm = sorted((RACINE / "user" / "apk" / "bonjour").glob("*.dasm"))
    dex = FIXTURES / "d07" / "bonjour.dex"
    lancer("python3", "tools/dexasm.py", *dasm, "--sortie", dex)
    base = [
        "python3",
        "tools/mkapk.py",
        "user/apk/bonjour/bonjour.toml",
        "--dex",
        dex,
        "--cles",
        FIXTURES / "d07" / "cles",
    ]
    lancer(*base, "--sortie", FIXTURES / "d07" / "ok.apk")
    lancer(*base, "--sortie", FIXTURES / "d07" / "nosig.apk", "--sans-signature")


def enregistrements(chemin):
    brut = chemin.read_bytes()
    off = 0
    while off + 12 <= len(brut):
        code, alen, blen = struct.unpack_from("<iII", brut, off)
        yield code, brut[off + 12 : off + 12 + alen]
        off += 12 + alen + blen


class Corpus:
    def __init__(self, cible):
        self.dossier = CORPUS / cible
        self.cible = cible
        self.vus = set()
        shutil.rmtree(self.dossier, ignore_errors=True)
        self.dossier.mkdir(parents=True)

    def ajouter(self, nom, donnees, plafond=None):
        plafond = plafond or TAILLE_MAX.get(self.cible, 1 << 30)
        empreinte = hashlib.sha256(donnees).digest()
        if len(donnees) > plafond or empreinte in self.vus:
            return
        self.vus.add(empreinte)
        (self.dossier / nom).write_bytes(donnees)


def corpus_zip():
    c = Corpus("zip")
    d = FIXTURES / "d01"
    c.ajouter("ok.zip", (d / "ok.zip").read_bytes())
    for i, (code, a) in enumerate(enregistrements(d / "cas.pack")):
        c.ajouter(f"cas{i:02d}-code{-code}.zip", a)
    c.ajouter("ok.apk", (FIXTURES / "d07" / "ok.apk").read_bytes(), 16384)
    c.ajouter("nosig.apk", (FIXTURES / "d07" / "nosig.apk").read_bytes(), 16384)


def corpus_inflate():
    c = Corpus("inflate")
    d = FIXTURES / "d01"
    for i, (code, a) in enumerate(enregistrements(d / "mauvais.pack")):
        c.ajouter(f"mauvais{i:02d}", a)
    pas = 0
    for i, (code, a) in enumerate(enregistrements(d / "deflate.pack")):
        if 0 < len(a) <= 1200:
            pas += 1
            if pas % 3 == 0:
                c.ajouter(f"bon{i:03d}", a)
    c.ajouter("stocke-a", b"\x01\x01\x00\xfe\xffa")
    c.ajouter("fixe-vide", b"\x03\x00")


def elf_minimal(type_elf, base, flags, filesz, memsz, entree=0x40, dyn=False):
    ph = struct.pack("<IIQQQQQQ", 1, flags, 0, base, base, filesz, memsz, 0x1000)
    nph = 1
    if dyn:
        ph += struct.pack("<IIQQQQQQ", 6474, 6, 0, base, base, 0, 0, 8)
        ph = ph[:56] + struct.pack("<IIQQQQQQ", 0x6474E551, 6, 0, 0, 0, 0, 0, 16)
        nph = 2
    ident = b"\x7fELF\x02\x01\x01" + b"\0" * 9
    eh = struct.pack(
        "<16sHHIQQQIHHHHHH", ident, type_elf, 62, 1, base + entree, 64, 0, 0, 64, 56, nph, 64, 0, 0
    )
    corps = (eh + ph).ljust(filesz, b"\x90")
    return corps


def elf_dynamique():
    taille, base = 0x3000, 0
    ident = b"\x7fELF\x02\x01\x01" + b"\0" * 9
    eh = struct.pack("<16sHHIQQQIHHHHHH", ident, 3, 62, 1, 0x800, 64, 0, 0, 64, 56, 6, 64, 0, 0)

    def ph(type_, flags, off, filesz, memsz):
        return struct.pack("<IIQQQQQQ", type_, flags, off, off, off, filesz, memsz, 0x1000)

    tables = [
        ph(1, 5, 0, 0x1000, 0x1000),
        ph(1, 4, 0x1000, 0x1000, 0x1000),
        ph(1, 6, 0x2000, 0x1000, 0x1800),
        ph(2, 6, 0x2000, 0x40, 0x40),
        ph(0x6474E552, 4, 0x2000, 0x1000, 0x1000),
        ph(0x6474E551, 6, 0, 0, 0),
    ]
    image = bytearray(taille)
    image[0 : len(eh)] = eh
    off = 64
    for t in tables:
        image[off : off + 56] = t
        off += 56
    dyn = [(7, 0x2100), (8, 48), (9, 24), (0, 0)]
    for i, (tag, val) in enumerate(dyn):
        struct.pack_into("<qQ", image, 0x2000 + 16 * i, tag, val)
    for i in range(2):
        struct.pack_into("<QQq", image, 0x2100 + 24 * i, 0x2200 + 8 * i, 8, 0x1000 * i)
    image[0x800] = 0x90
    return bytes(image)


def corpus_elf():
    c = Corpus("elf")
    c.ajouter("dyn-6-segments", elf_dynamique(), 0x4000)
    c.ajouter("dyn-min", elf_minimal(3, 0, 5, 0x80, 0x1000))
    c.ajouter("exec-min", elf_minimal(2, 0x400000, 5, 0x80, 0x1000))
    c.ajouter("exec-bss", elf_minimal(2, 0x400000, 5, 0x80, 0x20000))
    c.ajouter("dyn-stack", elf_minimal(3, 0, 5, 0x100, 0x1000, dyn=True))
    c.ajouter("refus-wx", elf_minimal(2, 0x400000, 7, 0x80, 0x1000))
    c.ajouter("refus-entree", elf_minimal(2, 0x400000, 5, 0x80, 0x1000, entree=0x2000))
    reel = RACINE / "tests" / "fuzz" / "elf_reel.bin"
    if reel.exists():
        c.ajouter("reel-strippe", reel.read_bytes(), 0x10000)


def entree_cpio(nom, mode, donnees=b"", ino=1, nlink=1):
    nom_nul = nom.encode() + b"\0"
    champs = (ino, mode, 0, 0, nlink, 1760000000, len(donnees), 0, 0, 0, 0, len(nom_nul), 0)
    brut = b"070701" + "".join(f"{v:08X}" for v in champs).encode() + nom_nul
    brut += b"\0" * (-len(brut) % 4)
    return brut + donnees + b"\0" * (-len(donnees) % 4)


def corpus_cpio():
    c = Corpus("cpio")
    fin = entree_cpio("TRAILER!!!", 0)
    d, f, ln = 0o040755, 0o100644, 0o120777
    c.ajouter("vide", b"")
    c.ajouter("trailer-seul", fin)
    c.ajouter("un-fichier", entree_cpio("bonjour", f, b"salut\n") + fin)
    valide = b"".join(
        [
            entree_cpio(".", d, nlink=2),
            entree_cpio("system", d, nlink=2),
            entree_cpio("system/bin/hello", 0o100755, b"bonjour\n"),
            entree_cpio("./vide", f),
            entree_cpio("x/y/z", f, b"zz"),
            entree_cpio("l" + "L" * 199, f, b"long"),
            entree_cpio("system/bin/hello", 0o100755, b"nouveau"),
            entree_cpio("lien", ln, b"system"),
            fin,
        ]
    )
    c.ajouter("valide-melange", valide)
    c.ajouter(
        "arbre",
        b"".join(
            entree_cpio(n, d if n.endswith("/") else f, b"x")
            for n in ["a/", "a/b/", "a/b/c", "a/d", "e"]
        )
        + fin,
    )
    c.ajouter(
        "noms-dangereux",
        b"".join(
            entree_cpio(n, f, b"x")
            for n in ["../etc", "a/../b", "/abs", "a//b", "a/./b", "\xff\xfe"]
        )
        + fin,
    )
    c.ajouter("tronque", valide[: len(valide) // 2])


def corpus_dex():
    c = Corpus("dex")
    c.ajouter("classes.dex", (FIXTURES / "d03" / "classes.dex").read_bytes())
    c.ajouter("bonjour.dex", (FIXTURES / "d07" / "bonjour.dex").read_bytes())


def corpus_axml():
    c = Corpus("axml")
    for chemin in sorted((FIXTURES / "d02").iterdir()):
        c.ajouter(chemin.name, chemin.read_bytes(), 8192)


FAT_FENETRE_FAT = 8
FAT_FENETRE_DONNEES = 32


def image_fat(chemin, spc, fichiers, bps=512):
    taille = 36 * 1024 * 1024 * max(1, spc)
    with open(chemin, "wb") as f:
        f.truncate(taille)
    lancer("mkfs.fat", "-F", "32", "-s", spc, "-S", bps, "-R", 32, "-f", 2, "-i", "56454c55", "-n", "FUZZ", chemin)
    dossiers = sorted({str(Path(n).parent) for n in fichiers if str(Path(n).parent) != "."})
    for d in dossiers:
        morceaux = Path(d).parts
        for i in range(1, len(morceaux) + 1):
            subprocess.run(["mmd", "-i", str(chemin), "::" + "/".join(morceaux[:i])], check=False, capture_output=True)
    tmp = FIXTURES / "fat-src"
    tmp.mkdir(parents=True, exist_ok=True)
    for nom, contenu in fichiers.items():
        src = tmp / "src.bin"
        src.write_bytes(contenu)
        lancer("mcopy", "-i", chemin, src, "::" + nom)


def compacter_fat(chemin):
    brut = chemin.read_bytes()
    bps, spc, rsvd, nfats = struct.unpack_from("<HBHB", brut, 11)
    fatsz = struct.unpack_from("<I", brut, 36)[0]
    fsinfo = struct.unpack_from("<H", brut, 48)[0]
    data_start = rsvd + nfats * fatsz
    parts = [brut[:512], brut[fsinfo * bps : fsinfo * bps + 512]]
    parts.append(brut[rsvd * bps : rsvd * bps + FAT_FENETRE_FAT * 512])
    parts.append(brut[data_start * bps : data_start * bps + FAT_FENETRE_DONNEES * 512])
    return b"".join(p.ljust(512 * (1 if i < 2 else (FAT_FENETRE_FAT if i == 2 else FAT_FENETRE_DONNEES)), b"\0") for i, p in enumerate(parts))


def corpus_fat():
    c = Corpus("fat")
    dossier = FIXTURES / "fat"
    dossier.mkdir(parents=True, exist_ok=True)
    arbre = {
        "BONJOUR.TXT": b"bonjour velum\n" * 8,
        "Un Nom Long Avec Espaces.txt": b"x" * 700,
        "system/bin/hello": bytes(range(256)) * 2,
        "system/etc/conf.d/reseau.conf": b"ip=10.0.0.2\n",
        "caf\u00e9 \u00e9t\u00e9.txt": b"utf8",
        "vide.dat": b"",
    }
    variantes = [("spc1", 1, arbre), ("spc2", 2, {"A.TXT": b"a" * 3000, "dir/B.TXT": b"b"}), ("spc8", 8, {"GRAND.BIN": b"g" * 9000})]
    for nom, spc, fichiers in variantes:
        image = dossier / f"{nom}.img"
        image_fat(image, spc, fichiers)
        c.ajouter(f"{nom}.fat", compacter_fat(image), 0x6000)
    image = dossier / "vierge.img"
    image_fat(image, 1, {})
    c.ajouter("vierge.fat", compacter_fat(image), 0x6000)


ACPI_BASE = 0xE0000


def table_acpi(sig, corps, rev=1):
    entete = struct.pack("<4sIBB6s8sIII", sig, 36 + len(corps), rev, 0, b"VELUM ", b"FUZZTBL ", 1, 0x4D4C4556, 1)
    brut = bytearray(entete + corps)
    brut[9] = (-sum(brut)) & 0xFF
    return bytes(brut)


def madt():
    corps = struct.pack("<II", 0xFEE00000, 1)
    corps += struct.pack("<BBBBI", 0, 8, 0, 0, 1) + struct.pack("<BBBBI", 0, 8, 1, 1, 1)
    corps += struct.pack("<BBBBII", 1, 12, 0, 0, 0xFEC00000, 0)
    corps += struct.pack("<BBBBIH", 2, 10, 0, 0, 2, 0)
    corps += struct.pack("<BBBBH", 4, 6, 0xFF, 0, 0) + b"\0"
    return table_acpi(b"APIC", corps, 3)


def fadt(dsdt):
    corps = bytearray(244 - 36)
    struct.pack_into("<I", corps, 40 - 36, 0)
    struct.pack_into("<H", corps, 46 - 36, 9)
    struct.pack_into("<I", corps, 64 - 36, 0x604)
    corps[89 - 36] = 2
    struct.pack_into("<I", corps, 112 - 36, 0x400)
    struct.pack_into("<Q", corps, 140 - 36, dsdt)
    struct.pack_into("<I", corps, 40 - 36, dsdt)
    return table_acpi(b"FACP", bytes(corps), 5)


def acpi_complet(xsdt=True):
    s5 = bytes([0x08, ord("_"), ord("S"), ord("5"), ord("_"), 0x12, 0x06, 0x04, 0x0A, 0x05, 0x0A, 0x05])
    dsdt = table_acpi(b"DSDT", s5, 2)
    hpet = table_acpi(b"HPET", b"\0" * 4 + struct.pack("<BBBBQ", 0, 64, 0, 0, 0xFED00000) + b"\0" * 4)
    mcfg = table_acpi(b"MCFG", b"\0" * 8 + struct.pack("<QHBBI", 0xE0000000, 0, 0, 255, 0))
    pos = {"rsdt": 0x40, "xsdt": 0x80, "facp": 0x200, "apic": 0x400, "hpet": 0x600, "mcfg": 0x700, "dsdt": 0x800}
    esize = 8 if xsdt else 4
    tables = [ACPI_BASE + pos[k] for k in ("facp", "apic", "hpet", "mcfg")]
    fmt = "<Q" if xsdt else "<I"
    racine = table_acpi(b"XSDT" if xsdt else b"RSDT", b"".join(struct.pack(fmt, t) for t in tables))
    img = bytearray(0x900)
    rsdp = struct.pack("<8sB6sBI", b"RSD PTR ", 0, b"VELUM ", 2 if xsdt else 0, ACPI_BASE + pos["rsdt"])
    if xsdt:
        rsdp += struct.pack("<IQBBBB", 36, ACPI_BASE + pos["xsdt"], 0, 0, 0, 0)
    brut = bytearray(rsdp)
    brut[8] = (-sum(brut[:20])) & 0xFF
    if xsdt:
        brut[32] = (-sum(brut)) & 0xFF
    img[0 : len(brut)] = brut
    img[pos["xsdt" if xsdt else "rsdt"] : pos["xsdt" if xsdt else "rsdt"] + len(racine)] = racine
    for nom, blob in (("facp", fadt(ACPI_BASE + pos["dsdt"])), ("apic", madt()), ("hpet", hpet), ("mcfg", mcfg), ("dsdt", dsdt)):
        img[pos[nom] : pos[nom] + len(blob)] = blob
    return bytes(img)


def corpus_acpi():
    c = Corpus("acpi")
    c.ajouter("xsdt-complet", acpi_complet(True))
    c.ajouter("rsdt-complet", acpi_complet(False))
    c.ajouter("rsdp-seul", acpi_complet(True)[:36])
    c.ajouter("aml-s5", bytes([0x10, 0x08, 0x08]) + b"_S5_" + bytes([0x12, 0x06, 0x04, 0x0A, 0x05, 0x0A, 0x05]))


CIBLES = {
    "zip": corpus_zip,
    "inflate": corpus_inflate,
    "elf": corpus_elf,
    "cpio": corpus_cpio,
    "dex": corpus_dex,
    "axml": corpus_axml,
    "fat": corpus_fat,
    "acpi": corpus_acpi,
}


def main():
    voulues = sys.argv[1:] or list(CIBLES)
    fixtures()
    for nom in voulues:
        CIBLES[nom]()
    total = sum(p.stat().st_size for p in CORPUS.rglob("*") if p.is_file())
    print(f"corpus : {total} octets sur {PLAFOND_TOTAL} autorises")
    return 0 if total <= PLAFOND_TOTAL else 1


if __name__ == "__main__":
    sys.exit(main())
