import re
import struct
import subprocess
import time

import mkimage
from vtest import VM

NAME = "intégrité : Limine refuse un noyau ou un initrd altéré (BLAKE2b)"
FIRMWARES = ("bios", "uefi")

CHEMIN_NOYAU = "/boot/kernel.elf"
CHEMIN_INITRD = "/boot/initrd.cpio"
CHEMIN_CONFIG = "/boot/limine/limine.conf"
DEMARRAGE = r"VelumOS boot ok"
BANNIERE_NOYAU = r"VelumOS \d+\.\d+, build "
SEQUENCE_ECRAN = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
DELAI_DEMARRAGE = 60
DELAI_REFUS = 30
ATTENTE_ABSENCE = 8
SHF_ALLOC = 0x2
SHT_NOBITS = 8
TAILLE_SECTION_MIN = 64
HEX128 = re.compile(r"#[0-9a-f]{128}")


def cible_esp(image):
    return f"{image}@@{mkimage.planifier(8).esp_debut}"


def lire_esp(image, chemin):
    commande = ["mcopy", "-i", cible_esp(image), f"::{chemin}", "-"]
    return subprocess.run(commande, capture_output=True, check=True).stdout


def ecrire_esp(image, chemin, donnees, source):
    source.write_bytes(donnees)
    commande = ["mcopy", "-o", "-i", cible_esp(image), str(source), f"::{chemin}"]
    subprocess.run(commande, capture_output=True, check=True)


def offset_inoffensif_noyau(elf):
    debut = struct.unpack_from("<Q", elf, 0x28)[0]
    taille_entree, nombre = struct.unpack_from("<HH", elf, 0x3A)
    candidats = []
    for i in range(nombre):
        base = debut + i * taille_entree
        genre = struct.unpack_from("<I", elf, base + 4)[0]
        drapeaux, _, offset, taille = struct.unpack_from("<QQQQ", elf, base + 8)
        if genre != SHT_NOBITS and not drapeaux & SHF_ALLOC and taille >= TAILLE_SECTION_MIN:
            candidats.append((taille, offset))
    assert candidats, "le noyau n'a aucune section non chargée à altérer"
    taille, offset = max(candidats)
    return offset + taille // 2


def offset_inoffensif_initrd(cpio):
    rang = cpio.find(b"NAME=VelumOS")
    assert rang >= 0, "os-release introuvable dans l'initrd"
    return rang + len(b"NAME=")


def alterer(donnees, rang):
    modifie = bytearray(donnees)
    modifie[rang] ^= 0x01
    return bytes(modifie)


def config_variante(conf, serie, sans_empreinte_de=None):
    texte = conf.decode()
    if sans_empreinte_de is not None:
        texte = re.sub(rf"({re.escape(sans_empreinte_de)})#[0-9a-f]{{128}}", r"\1", texte)
    lignes = texte.splitlines()
    if serie:
        lignes.insert(0, "serial: yes")
    return ("\n".join(lignes) + "\n").encode()


def fabriquer_variante(vm, nom, noyau=None, initrd=None, conf=None):
    image = vm.dossier / f"{vm.nom}-{nom}.img"
    image.write_bytes(vm.disque.read_bytes())
    source = vm.dossier / f"{vm.nom}-{nom}.source"
    for chemin, donnees in ((CHEMIN_NOYAU, noyau), (CHEMIN_INITRD, initrd), (CHEMIN_CONFIG, conf)):
        if donnees is not None:
            ecrire_esp(image, chemin, donnees, source)
    source.unlink(missing_ok=True)
    return image


def machine_pour(vm, image, nom):
    return VM(
        image,
        vm.dossier,
        nom=f"{vm.nom}-{nom}",
        uefi=vm.uefi,
        memoire=vm.memoire,
        smp=vm.smp,
        extra=vm.extra,
    )


def doit_demarrer(vm, image, nom):
    try:
        with machine_pour(vm, image, nom) as machine:
            machine.attendre(DEMARRAGE, delai=DELAI_DEMARRAGE)
    finally:
        image.unlink(missing_ok=True)


def serie_nette(machine):
    return SEQUENCE_ECRAN.sub("", machine.serie())


def doit_etre_refuse(vm, image, nom, fichier):
    try:
        verifier_refus(vm, image, nom, fichier)
    finally:
        image.unlink(missing_ok=True)


def verifier_refus(vm, image, nom, fichier):
    refus_attendu = rf"PANIC: Blake2b hash for URI `boot\(\):{re.escape(fichier)}` does not match"
    with machine_pour(vm, image, nom) as machine:
        fin = time.monotonic() + DELAI_REFUS
        refus = None
        while refus is None and time.monotonic() < fin:
            texte = serie_nette(machine)
            assert not re.search(DEMARRAGE, texte), f"{nom} : le noyau a démarré\n{texte[-1500:]}"
            refus = re.search(refus_attendu, texte)
            if machine.processus.poll() is not None:
                break
            time.sleep(0.1)
        texte = serie_nette(machine)
        assert refus, f"{nom} : aucun refus de Limine sur la série\n{texte[-1500:]}"
        time.sleep(ATTENTE_ABSENCE)
        texte = serie_nette(machine)
        assert not re.search(BANNIERE_NOYAU, texte), f"{nom} : kmain atteint\n{texte[-1500:]}"
        assert not re.search(DEMARRAGE, texte), f"{nom} : le noyau a démarré\n{texte[-1500:]}"
        assert machine.processus.poll() is None, f"{nom} : la machine s'est arrêtée ou relancée"


def run(vm):
    vm.attendre(DEMARRAGE, delai=DELAI_DEMARRAGE)
    noyau = lire_esp(vm.disque, CHEMIN_NOYAU)
    initrd = lire_esp(vm.disque, CHEMIN_INITRD)
    conf = lire_esp(vm.disque, CHEMIN_CONFIG)
    assert len(HEX128.findall(conf.decode())) == 2, conf.decode()
    attendus = (mkimage.empreinte_blake2b(noyau), mkimage.empreinte_blake2b(initrd))
    assert all(f"#{h}" in conf.decode() for h in attendus), conf.decode()
    noyau_altere = alterer(noyau, offset_inoffensif_noyau(noyau))
    initrd_altere = alterer(initrd, offset_inoffensif_initrd(initrd))
    assert noyau_altere != noyau and initrd_altere != initrd
    sans_noyau = config_variante(conf, True, "boot():/boot/kernel.elf")
    sans_initrd = config_variante(conf, True, "boot():/boot/initrd.cpio")
    avec_hash = config_variante(conf, True)
    controle = fabriquer_variante(vm, "temoin-noyau", noyau=noyau_altere, conf=sans_noyau)
    doit_demarrer(vm, controle, "temoin-noyau")
    controle = fabriquer_variante(vm, "temoin-initrd", initrd=initrd_altere, conf=sans_initrd)
    doit_demarrer(vm, controle, "temoin-initrd")
    refuse = fabriquer_variante(vm, "noyau", noyau=noyau_altere, conf=avec_hash)
    doit_etre_refuse(vm, refuse, "noyau", CHEMIN_NOYAU)
    refuse = fabriquer_variante(vm, "initrd", initrd=initrd_altere, conf=avec_hash)
    doit_etre_refuse(vm, refuse, "initrd", CHEMIN_INITRD)
