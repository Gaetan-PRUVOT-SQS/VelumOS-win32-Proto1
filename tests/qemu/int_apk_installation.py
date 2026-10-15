import re
import shutil
import subprocess
import sys
from pathlib import Path

NAME = (
    "APK : installation sur /data (valide, altéré, non signé, autre signataire), "
    "appli installée lancée, APK altéré refusé au lancement"
)
CMDLINE = "quiet init=/system/bin/apkessai"
FIRMWARES = ("bios",)

RACINE = Path(__file__).resolve().parent.parent.parent
TAILLE_OCTETS = 64 * 1024 * 1024
OUTILS = ("mformat", "mcopy", "mmd", "mdir", "fsck.vfat", "openssl")
PAQUET = "com.velum.bonjour"
OCTET_ALTERE = 2000
REFUS_SIGNATURE = r"refusé : La signature de l'application est absente ou invalide\."
LIGNE_REGISTRE = re.compile(
    rf"^{re.escape(PAQUET)};1;Bonjour APK;Lcom/velum/bonjour/Principale;;[0-9a-f]{{64}}$"
)
LIGNES_SANS_GRAVITE = ("fsck.fat", "Leaving filesystem unchanged", "Auto-correcting")
IMAGE = []


def _dossier():
    argv = sys.argv
    if "--out" in argv and argv.index("--out") + 1 < len(argv):
        return Path(argv[argv.index("--out") + 1]).resolve() / "int_apk_donnees"
    return RACINE / "build" / "apk" / "qemu" / "int_apk_donnees"


def _outil(nom):
    chemin = shutil.which(nom) or shutil.which(nom, path="/usr/sbin:/sbin")
    assert chemin, f"{nom} introuvable : installer mtools, dosfstools et openssl"
    return chemin


def _lancer(*commande):
    resultat = subprocess.run(commande, capture_output=True, text=True, check=False, cwd=RACINE)
    assert resultat.returncode == 0, f"{commande[0]} : {resultat.stdout}{resultat.stderr}"
    return resultat.stdout


def _mkapk(dossier, nom, cles, *options):
    sortie = dossier / nom
    _lancer(
        sys.executable,
        "tools/mkapk.py",
        "user/apk/bonjour/bonjour.toml",
        "--dex",
        str(dossier / "bonjour.dex"),
        "--sortie",
        str(sortie),
        "--cles",
        str(dossier / cles),
        *options,
    )
    return sortie


def fabriquer_apk(dossier):
    sources = sorted(str(p) for p in (RACINE / "user" / "apk" / "bonjour").glob("*.dasm"))
    _lancer(sys.executable, "tools/dexasm.py", *sources, "--sortie", str(dossier / "bonjour.dex"))
    return [
        _mkapk(dossier, "ok.apk", "cles-a"),
        _mkapk(dossier, "altere.apk", "cles-a", "--alterer", str(OCTET_ALTERE)),
        _mkapk(dossier, "nonsigne.apk", "cles-a", "--sans-signature"),
        _mkapk(dossier, "autrecle.apk", "cles-b"),
    ]


def creer_image(dossier):
    dossier.mkdir(parents=True, exist_ok=True)
    image = dossier / "int_apk.img"
    image.unlink(missing_ok=True)
    with image.open("wb") as flux:
        flux.truncate(TAILLE_OCTETS)
    base = ("-i", str(image))
    _lancer(_outil("mformat"), *base, "-F", "-v", "VELUMDATA", "::")
    _lancer(_outil("mmd"), *base, "::depot")
    for apk in fabriquer_apk(dossier):
        _lancer(_outil("mcopy"), *base, str(apk), f"::depot/{apk.name}")
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


def compter(vm, motif):
    return len(re.findall(motif, vm.serie()))


def verifier_disque(image):
    dossier = _dossier()
    relu = dossier / "installe_relu.apk"
    relu.unlink(missing_ok=True)
    _lancer(_outil("mcopy"), "-n", "-i", str(image), f"::apps/{PAQUET}.apk", str(relu))
    assert relu.read_bytes() == (dossier / "ok.apk").read_bytes(), (
        "l'APK installé diffère de l'APK déposé"
    )
    registre = dossier / "paquets_relu.txt"
    registre.unlink(missing_ok=True)
    _lancer(_outil("mcopy"), "-n", "-i", str(image), "::apps/paquets", str(registre))
    lignes = [ligne for ligne in registre.read_text(encoding="utf-8").splitlines() if ligne]
    assert len(lignes) == 1 and LIGNE_REGISTRE.match(lignes[0]), f"registre inattendu : {lignes}"
    noms = _lancer(_outil("mdir"), "-b", "-i", str(image), "::apps").split()
    assert sorted(noms) == [f"::/apps/{PAQUET}.apk", "::/apps/paquets"], (
        f"fichiers inattendus sous /data/apps : {noms}"
    )
    resultat = subprocess.run(
        [_outil("fsck.vfat"), "-n", str(image)], capture_output=True, text=True, check=False
    )
    sortie = resultat.stdout + resultat.stderr
    autres = [
        ligne.strip()
        for ligne in sortie.splitlines()
        if ligne.strip()
        and not ligne.strip().startswith(LIGNES_SANS_GRAVITE)
        and " files, " not in ligne
    ]
    assert resultat.returncode == 0 and not autres, f"fsck.vfat : {sortie}"


def run(vm):
    for outil in OUTILS:
        _outil(outil)
    vm.attendre(r"vfs: /data monté sur vda", delai=120)
    vm.attendre(r"APKESSAI start", delai=60)
    vm.attendre(r"APKESSAI etape 7 affichee", delai=120)
    assert compter(vm, rf"apkinst: {re.escape(PAQUET)} installé") == 2, vm.serie()[-3000:]
    assert compter(vm, rf"apkinst: {REFUS_SIGNATURE}") == 2, vm.serie()[-3000:]
    assert compter(vm, r"apkinst: refusé : signataire différent de la version installée") == 1
    assert compter(vm, r"apkinst: refusé : fichier introuvable") == 1
    vm.attendre(rf"apkrun: {re.escape(PAQUET)} signature ok", delai=10)
    vm.attendre(rf"apkrun: {re.escape(PAQUET)} prêt", delai=10)
    vm.capture("appli_installee.ppm").enregistrer_png(vm.dossier / "appli_installee.png")
    vm.attendre(r"APKESSAI etape 8 affichee", delai=60)
    vm.attendre(rf"apkrun: \S+altere\.apk {REFUS_SIGNATURE}", delai=10)
    vm.capture("appli_refusee.ppm").enregistrer_png(vm.dossier / "appli_refusee.png")
    vm.attendre(r"APKESSAI PASS 8 etapes", delai=60)
    texte = vm.serie()
    assert "PANIC:" not in texte and "APKESSAI FAIL" not in texte, texte[-3000:]
    assert compter(vm, r"apkrun: \S+ prêt") == 1, "seule l'appli valide doit démarrer"
    vm.processus.kill()
    vm.processus.wait(timeout=30)
    verifier_disque(IMAGE[0])
