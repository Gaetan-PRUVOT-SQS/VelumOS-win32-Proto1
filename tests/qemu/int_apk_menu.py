import re
import time

NAME = "APK : appli préinstallée lancée du menu Démarrer, trois clics comptés, fermeture"
CMDLINE = "quiet"
FIRMWARES = ("bios", "uefi")

LARGEUR = 1024
PAS = 100
PAUSE = 0.12
TUILE = (768, 323)
BOUTON_DEMARRER = (50, 753)
TOUS_LES_PROGRAMMES = (87, 589)
ENTREE_APK = (60, 619)
BOUTON_COMPTER = (390, 227)
COIN_FENETRE = (190, 300)
COULEUR_FENETRE = (0xEC, 0xE9, 0xD8)
PAQUET = "com.velum.bonjour"


def placer(vm, x, y):
    for _ in range(LARGEUR // 200 + 2):
        vm.souris_deplacer(-200, -200)
        time.sleep(PAUSE)
    reste_x, reste_y = x, y
    while reste_x > 0 or reste_y > 0:
        pas_x, pas_y = min(PAS, reste_x), min(PAS, reste_y)
        vm.souris_deplacer(pas_x, pas_y)
        time.sleep(PAUSE)
        reste_x -= pas_x
        reste_y -= pas_y


def cliquer(vm, x, y):
    placer(vm, x, y)
    vm.souris_bouton(1)
    time.sleep(0.1)
    vm.souris_bouton(0)
    time.sleep(0.4)


def fenetre_ouverte(image):
    rouge, vert, bleu = image.pixel(*COIN_FENETRE)
    return max(abs(a - b) for a, b in zip((rouge, vert, bleu), COULEUR_FENETRE, strict=True)) <= 8


def compter(vm, motif):
    return len(re.findall(motif, vm.serie()))


def run(vm):
    vm.attendre(r"logon: écran de connexion prêt", delai=120)
    cliquer(vm, *TUILE)
    vm.attendre(r"shell: bureau prêt", delai=60)
    cliquer(vm, *BOUTON_DEMARRER)
    cliquer(vm, *TOUS_LES_PROGRAMMES)
    sous_menu = vm.capture("sous_menu.ppm")
    sous_menu.enregistrer_png(vm.dossier / f"sous_menu-{vm.nom}.png")
    cliquer(vm, *ENTREE_APK)
    vm.attendre(rf"apkrun: {re.escape(PAQUET)} signature ok", delai=30)
    vm.attendre(r"Bonjour: onCreate", delai=30)
    vm.attendre(rf"apkrun: {re.escape(PAQUET)} prêt", delai=30)
    ouverte = vm.capture_quand(fenetre_ouverte, delai=10, nom="appli.ppm")
    assert fenetre_ouverte(ouverte), "la fenêtre de l'appli doit être affichée"
    for rang in (1, 2, 3):
        cliquer(vm, *BOUTON_COMPTER)
        vm.attendre(rf"Bonjour: Clics : {rang}\b", delai=20)
    apres = vm.capture("appli_trois_clics.ppm")
    apres.enregistrer_png(vm.dossier / f"appli_trois_clics-{vm.nom}.png")
    assert apres.differences(ouverte) > 20, "le texte de l'appli doit avoir changé"
    vm.touche("alt-f4")
    fermee = vm.capture_quand(lambda image: not fenetre_ouverte(image), delai=10, nom="fermee.ppm")
    assert not fenetre_ouverte(fermee), "Alt+F4 doit fermer la fenêtre de l'appli"
    texte = vm.serie()
    assert "PANIC:" not in texte, texte[-2000:]
    assert "refusé" not in texte, texte[-2000:]
    assert compter(vm, r"Bonjour: Clics : \d+") == 3, "trois clics, trois lignes de journal"
