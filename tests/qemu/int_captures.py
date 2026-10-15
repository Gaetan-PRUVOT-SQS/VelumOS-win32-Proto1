import time

NAME = "intégration : captures d'écran du parcours (README)"
CMDLINE = "quiet"
FIRMWARES = ("bios",)
PAUSE = 0.12
LARGEUR = 1024
TUILE = (768, 323)
ICONE_BONJOUR = (44, 182)
BOUTON_DEMARRER = (50, 753)


def placer(vm, x, y):
    for _ in range(LARGEUR // 200 + 2):
        vm.souris_deplacer(-200, -200)
        time.sleep(PAUSE)
    reste_x, reste_y = x, y
    while reste_x > 0 or reste_y > 0:
        pas_x, pas_y = min(100, reste_x), min(100, reste_y)
        vm.souris_deplacer(pas_x, pas_y)
        time.sleep(PAUSE)
        reste_x -= pas_x
        reste_y -= pas_y


def cliquer(vm, x, y, fois=1):
    placer(vm, x, y)
    for _ in range(fois):
        vm.souris_bouton(1)
        time.sleep(0.05)
        vm.souris_bouton(0)
        time.sleep(0.08)
    time.sleep(0.4)


def enregistrer(vm, nom):
    image = vm.capture(f"{nom}.ppm")
    image.enregistrer_png(vm.dossier / f"{nom}.png")


def run(vm):
    vm.attendre(r"logon: écran de connexion prêt", delai=120)
    time.sleep(1)
    enregistrer(vm, "connexion")
    cliquer(vm, *TUILE)
    vm.attendre(r"shell: bureau prêt", delai=60)
    time.sleep(3)
    enregistrer(vm, "bureau")
    cliquer(vm, *ICONE_BONJOUR, fois=2)
    vm.attendre(r"hello", delai=20)
    time.sleep(2)
    enregistrer(vm, "fenetre-bonjour")
    cliquer(vm, *BOUTON_DEMARRER)
    time.sleep(1)
    enregistrer(vm, "menu-demarrer")
