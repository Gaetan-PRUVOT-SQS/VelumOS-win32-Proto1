import re
import time

NAME = "intégration : mort du serveur de fenêtres, relance par init, session rouverte"
CMDLINE = "quiet init.mort-winsrv"
MEMOIRE = "256M"
FIRMWARES = ("bios", "uefi")

LARGEUR = 1024
HAUTEUR = 768
HAUTEUR_BARRE = 30
BARRE = (0, HAUTEUR - HAUTEUR_BARRE, LARGEUR, HAUTEUR_BARRE)
BLEU_BARRE = (0x24, 0x5E, 0xDC)
TOLERANCE_BLEU = 40
PART_BLEUE_MINI = 0.30
CARTE_COIN = (592, 283)
COULEUR_CARTE = (0xEC, 0xE9, 0xD8)


def compter(vm, motif):
    return len(re.findall(motif, vm.serie()))


def attendre_occurrences(vm, motif, nombre, delai, etape):
    limite = time.monotonic() + delai
    while time.monotonic() < limite:
        if compter(vm, motif) >= nombre:
            return
        assert vm.processus.poll() is None, f"{etape} : QEMU s'est arrêté"
        time.sleep(0.2)
    fin = "\n".join(vm.serie().splitlines()[-25:])
    raise AssertionError(
        f"{etape} : « {motif} » vu {compter(vm, motif)} fois, {nombre} attendu(s)\n{fin}"
    )


def carte_de_connexion(image):
    rouge, vert, bleu = image.pixel(*CARTE_COIN)
    return max(abs(a - b) for a, b in zip((rouge, vert, bleu), COULEUR_CARTE, strict=True)) <= 8


def barre_des_taches(image):
    bleus = image.compter(BARRE, BLEU_BARRE, TOLERANCE_BLEU)
    return bleus > PART_BLEUE_MINI * LARGEUR * HAUTEUR_BARRE


def ouvrir_session(vm, rang, etape):
    attendre_occurrences(vm, r"logon: écran de connexion prêt", rang, 120, f"{etape} connexion")
    connexion = vm.capture_stable(nom=f"connexion{rang}.ppm")
    connexion.enregistrer_png(vm.dossier / f"connexion{rang}.png")
    assert carte_de_connexion(connexion), f"{etape} : carte de connexion absente de l'écran"
    assert not barre_des_taches(connexion), f"{etape} : barre des tâches sur l'écran de connexion"
    vm.touche("ret")
    attendre_occurrences(vm, r"logon: session ouverte", rang, 60, f"{etape} ouverture")
    attendre_occurrences(vm, r"shell: bureau prêt", rang, 60, f"{etape} bureau")
    bureau = vm.capture_quand(barre_des_taches, delai=10, nom=f"bureau{rang}.ppm")
    bureau.enregistrer_png(vm.dossier / f"bureau{rang}.png")
    assert barre_des_taches(bureau), f"{etape} : barre des tâches absente du bureau"


def run(vm):
    vm.attendre(r"init: essai mort-winsrv armé", delai=120)
    ouvrir_session(vm, 1, "étape 1, première session")

    vm.attendre(r"init: essai mort-winsrv, arrêt de /system/bin/winsrv \(0\)", delai=60)
    vm.attendre(r"init: /system/bin/winsrv terminé \(code 99\)", delai=30)
    attendre_occurrences(vm, r"init: /system/bin/winsrv lancé", 2, 30, "étape 2, relance par init")
    attendre_occurrences(vm, r"winsrv: pret", 2, 30, "étape 3, serveur relancé prêt")

    ouvrir_session(vm, 2, "étape 4, session après relance")

    texte = vm.serie()
    assert "PANIC:" not in texte, "étape 5 : panique du noyau"
    assert "abandon" not in texte, "étape 5 : init a abandonné un service"
    assert compter(vm, r"essai mort-winsrv, arrêt") == 1, "étape 5 : un seul arrêt provoqué"
