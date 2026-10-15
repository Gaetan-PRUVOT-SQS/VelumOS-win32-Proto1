import time

NAME = "a13 : écran d'arrêt bleu (fault=panic)"
CMDLINE = "selftest fault=panic init=/aucun"

BLEU_ARRET = (0, 0, 170)


def run(vm):
    vm.attendre(r"PANIC:", delai=90)
    time.sleep(1.0)
    assert vm.processus.poll() is None, "la machine doit rester figée sur l'écran d'arrêt"
    image = vm.capture()
    larg, haut = image.largeur, image.hauteur
    bleus = image.compter((0, 0, larg, haut), BLEU_ARRET, 2)
    assert bleus * 5 > larg * haut * 4, f"moins de 80 % de pixels #0000AA : {bleus}/{larg * haut}"
    blancs = image.compter((0, 0, larg, haut), (255, 255, 255), 2)
    assert blancs > 100, "le texte blanc de l'écran d'arrêt est absent"
    serie = vm.serie()
    assert "panne volontaire" in serie, "message de panique absent de la série"
