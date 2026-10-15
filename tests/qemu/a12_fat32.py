import shutil
import subprocess
import sys
from pathlib import Path

NAME = (
    "FAT32 : image mtools montée en virtio-blk, lecture, écriture, contrôle par mtools et fsck.vfat"
)
CMDLINE = "selftest exit"
FIRMWARES = ("bios",)

RACINE = Path(__file__).resolve().parent.parent.parent
TAILLE_OCTETS = 64 * 1024 * 1024
LONGUEUR_MOTIF = 70000
FICHIER_ECRIT = "::velum/ecrit par velum.txt"
LISTE_ATTENDUE = ["::/velum/ecrit par velum.txt"]
LIGNES_SANS_GRAVITE = ("fsck.fat", "Leaving filesystem unchanged", "Auto-correcting")
OUTILS = ("mformat", "mcopy", "mmd", "mdir", "fsck.vfat")
IMAGE = []


def _dossier():
    argv = sys.argv
    if "--out" in argv and argv.index("--out") + 1 < len(argv):
        return Path(argv[argv.index("--out") + 1]).resolve() / "a12_donnees"
    return RACINE / "build" / "a12" / "qemu" / "a12_donnees"


def _outil(nom):
    chemin = shutil.which(nom) or shutil.which(nom, path="/usr/sbin:/sbin")
    assert chemin, f"{nom} introuvable : installer mtools et dosfstools"
    return chemin


def _lancer(*commande):
    resultat = subprocess.run(commande, capture_output=True, text=True, check=False)
    assert resultat.returncode == 0, f"{commande[0]} : {resultat.stdout}{resultat.stderr}"
    return resultat.stdout


def motif():
    return bytes((i * 7 + i // 256) % 251 for i in range(LONGUEUR_MOTIF))


def creer_image(dossier):
    dossier.mkdir(parents=True, exist_ok=True)
    image = dossier / "a12_fat32.img"
    image.unlink(missing_ok=True)
    with image.open("wb") as flux:
        flux.truncate(TAILLE_OCTETS)
    court = dossier / "LISEZMOI.TXT"
    court.write_bytes(b"VelumOS FAT32\n")
    long = dossier / "Fichier au nom long.txt"
    long.write_bytes(b"nom long\n")
    base = ("-i", str(image))
    _lancer(_outil("mformat"), *base, "-F", "-v", "VELUMDATA", "::")
    _lancer(_outil("mcopy"), *base, str(court), "::LISEZMOI.TXT")
    _lancer(_outil("mmd"), *base, "::Dossier long")
    _lancer(_outil("mcopy"), *base, str(long), "::Dossier long/Fichier au nom long.txt")
    return image


class _Disque:
    def __iter__(self):
        image = creer_image(_dossier())
        IMAGE[:] = [image]
        return iter(
            [
                "-drive",
                f"file={image},if=none,id=d1,format=raw",
                "-device",
                "virtio-blk-pci,drive=d1,disable-legacy=on,addr=0x10",
            ]
        )


QEMU_EXTRA = _Disque()


def attendre_arret(vm, delai):
    try:
        vm.processus.wait(timeout=delai)
    except subprocess.TimeoutExpired as erreur:
        raise AssertionError("QEMU devait quitter à la fin des autotests") from erreur


def verifier_fichier_ecrit(image):
    sortie = _dossier() / "relu_par_mtools.bin"
    sortie.unlink(missing_ok=True)
    _lancer(_outil("mcopy"), "-n", "-i", str(image), FICHIER_ECRIT, str(sortie))
    relu = sortie.read_bytes()
    assert len(relu) == LONGUEUR_MOTIF, f"taille relue {len(relu)}, attendue {LONGUEUR_MOTIF}"
    assert relu == motif(), "le contenu écrit par le noyau diffère du motif attendu"


def verifier_dossier_nettoye(image):
    liste = _lancer(_outil("mdir"), "-b", "-i", str(image), "::velum").split("\n")
    noms = [ligne.strip() for ligne in liste if ligne.strip()]
    assert noms == LISTE_ATTENDUE, (
        f"le dossier velum devait ne contenir que le fichier écrit : {noms}"
    )


def anomalies_fsck(sortie):
    lignes = [ligne.strip() for ligne in sortie.splitlines() if ligne.strip()]
    return [
        ligne
        for ligne in lignes
        if not ligne.startswith(LIGNES_SANS_GRAVITE) and " files, " not in ligne
    ]


def verifier_fsck(image):
    resultat = subprocess.run(
        [_outil("fsck.vfat"), "-n", str(image)], capture_output=True, text=True, check=False
    )
    sortie = resultat.stdout + resultat.stderr
    autres = anomalies_fsck(sortie)
    assert not autres, f"anomalies de structure FAT32 : {autres}"
    assert resultat.returncode == 0, f"fsck.vfat code {resultat.returncode}\n{sortie}"


def run(vm):
    for outil in OUTILS:
        _outil(outil)
    vm.attendre(r"vfs: fat32 sur /data, \d+ clusters", delai=120)
    vm.attendre(r"vfs: /data monté sur vda", delai=30)
    vm.attendre(r"vfs: autotest data ok", delai=240)
    vm.attendre(r"\[TEST\] vfs \.\.\. OK", delai=30)
    vm.attendre(r"SELFTESTS PASS \d+", delai=240)
    attendre_arret(vm, delai=60)
    image = IMAGE[0]
    verifier_fichier_ecrit(image)
    verifier_dossier_nettoye(image)
    verifier_fsck(image)
