NAME = "a13 : changement de mode Bochs/QEMU (DISPI)"
CMDLINE = "selftest dispmode=1280x720 init=/aucun"


def run(vm):
    vm.attendre(r"SELFTESTS PASS \d+", delai=90)
    image = vm.capture()
    assert (image.largeur, image.hauteur) == (1280, 720), (
        f"la capture doit faire 1280x720, obtenu {image.largeur}x{image.hauteur}"
    )
    noirs = image.compter((0, 0, 1280, 720), (0, 0, 0), 2)
    assert noirs * 10 >= 1280 * 720 * 7, "l'écran de démarrage doit être redessiné en noir"
    assert "display: mode 1280x720" in vm.serie(), "journal du changement de mode absent"
