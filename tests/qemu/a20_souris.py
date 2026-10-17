import subprocess
import time

NAME = "bureau : connexion et menu Démarrer à la souris, extinction"
CMDLINE = "quiet"
MEMOIRE = "256M"

LARGEUR = 1024
HAUTEUR = 768
PAS = 100
PAUSE = 0.12
CARTE_X = (LARGEUR - 380) // 2
CARTE_Y = 311
TUILE = (CARTE_X + 190, CARTE_Y + 28 + 65)
BOUTON_DEMARRER = (12 + 8 + 21, HAUTEUR - 12 - 56 + 8 + 21)
LANCEUR = (12, HAUTEUR - 80 - 289, 540, 289)
ETEINDRE_MENU = (LANCEUR[0] + LANCEUR[2] - 18 - 18, LANCEUR[1] + LANCEUR[3] - 18 - 18)
PIXELS_MENU_MINI = 60_000


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
    time.sleep(0.3)


def run(vm):
    vm.attendre(r"logon: écran de connexion prêt", delai=120)
    cliquer(vm, *TUILE)
    vm.attendre(r"shell: bureau prêt", delai=60)
    bureau = vm.capture("bureau_souris.ppm")
    cliquer(vm, *BOUTON_DEMARRER)
    menu = vm.capture("menu_souris.ppm")
    assert menu.differences(bureau) > PIXELS_MENU_MINI, "le clic sur Démarrer doit ouvrir le menu"
    cliquer(vm, *ETEINDRE_MENU)
    vm.touche("ret")
    try:
        vm.processus.wait(timeout=60)
    except subprocess.TimeoutExpired as erreur:
        raise AssertionError("QEMU devait quitter après « Arrêter »") from erreur
