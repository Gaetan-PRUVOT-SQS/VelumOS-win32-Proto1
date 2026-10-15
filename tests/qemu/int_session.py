import time

NAME = "intégration : session jusqu'à l'écran de connexion"
CMDLINE = "quiet"
FIRMWARES = ("bios",)


def run(vm):
    vm.attendre(r"init: démarrage de la session", delai=60)
    time.sleep(8)
    image = vm.capture("session.ppm")
    image.enregistrer_png(vm.dossier / "session.png")
    texte = vm.serie()
    assert "PANIC:" not in texte
