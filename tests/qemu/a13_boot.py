NAME = "a13 : écran de démarrage noir avec barre de progression bleue"
CMDLINE = "quiet init=/aucun"


def est_bleu(pixel):
    rouge, vert, bleu = pixel
    return bleu >= 150 and bleu >= rouge + 60 and bleu >= vert + 20


def compter_bleus(image, rect):
    x0, y0, largeur, hauteur = rect
    total = 0
    for y in range(y0, y0 + hauteur, 1):
        for x in range(x0, x0 + largeur, 1):
            if est_bleu(image.pixel(x, y)):
                total += 1
    return total


def run(vm):
    vm.attendre(r"VelumOS boot ok", delai=60)
    image = vm.capture()
    larg, haut = image.largeur, image.hauteur
    assert larg >= 640 and haut >= 480, f"capture trop petite : {larg}x{haut}"
    noirs = image.compter((0, 0, larg, haut), (0, 0, 0), 2)
    assert noirs * 10 >= larg * haut * 7, f"fond noir insuffisant : {noirs}/{larg * haut}"
    coin = image.compter((0, 0, larg // 8, haut // 8), (0, 0, 0), 2)
    assert coin == (larg // 8) * (haut // 8), "le coin haut gauche doit rester noir"
    bleus = compter_bleus(image, (larg // 8, haut // 2, larg * 3 // 4, haut // 2))
    assert bleus >= 30, f"barre de progression bleue absente ({bleus} pixels bleus)"
