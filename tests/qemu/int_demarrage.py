import time

NAME = "intégration : écran de démarrage (README)"
CMDLINE = "quiet init=/aucun"
FIRMWARES = ("bios",)


def run(vm):
    vm.attendre(r"VelumOS boot ok", delai=60)
    time.sleep(0.5)
    vm.capture("demarrage.ppm").enregistrer_png(vm.dossier / "demarrage.png")
