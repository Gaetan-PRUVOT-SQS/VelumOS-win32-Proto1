NAME = "a13 : console noyau en mode verbose"
CMDLINE = "verbose init=/aucun"

GRIS_CONSOLE = (192, 192, 192)


def run(vm):
    vm.attendre(r"VelumOS boot ok", delai=60)
    image = vm.capture()
    larg, haut = image.largeur, image.hauteur
    texte = image.compter((0, 0, larg, haut // 2), GRIS_CONSOLE, 0)
    assert texte >= 500, f"aucun texte de console visible ({texte} pixels gris clair)"
    noirs = image.compter((0, 0, larg, haut), (0, 0, 0), 2)
    assert noirs * 10 >= larg * haut * 8, "le fond de la console doit rester noir"
    bas = image.compter((0, haut - haut // 8, larg, haut // 8), (0, 0, 0), 2)
    assert bas * 10 >= (larg * (haut // 8)) * 9, "le bas de l'écran ne doit pas être écrit"
