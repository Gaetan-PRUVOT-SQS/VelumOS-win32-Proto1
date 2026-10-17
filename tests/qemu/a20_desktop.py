import subprocess

NAME = "bureau : connexion au clavier, dock en verre, lanceur, extinction"
CMDLINE = "quiet"
MEMOIRE = "256M"

LARGEUR = 1024
HAUTEUR = 768
MARGE_DOCK = 12
HAUTEUR_DOCK = 56
DOCK = (MARGE_DOCK, HAUTEUR - MARGE_DOCK - HAUTEUR_DOCK, LARGEUR - 2 * MARGE_DOCK, HAUTEUR_DOCK)
ORBE = (DOCK[0] + 8, DOCK[1] + (HAUTEUR_DOCK - 42) // 2, 42, 42)
BANDE_SOUS_DOCK = (0, HAUTEUR - MARGE_DOCK + 1, LARGEUR, MARGE_DOCK - 1)
LANCEUR = (12, HAUTEUR - 80 - 289, 540, 289)
HORS_LANCEUR = (
    LANCEUR[0] + LANCEUR[2] + 8,
    0,
    LARGEUR - LANCEUR[0] - LANCEUR[2] - 8,
    LANCEUR[1] - 8,
)
CARTE = ((LARGEUR - 380) // 2, 311, 380, 290)
HORLOGE = (CARTE[0], 100, CARTE[2], 120)
ENCRE = (0x14, 0x18, 0x24)
TOLERANCE_ENCRE = 70
PIXELS_ENCRE_HORLOGE_MINI = 1500
PART_DOCK_MODIFIEE_MINI = 0.60
PART_LANCEUR_MODIFIEE_MINI = 0.50
PIXELS_HORS_ZONE_MAXI = 400
PIXELS_MENU_FERME_MAXI = 400


def modifies(image, autre, rect, tolerance=6):
    x0, y0, largeur, hauteur = rect
    total = 0
    for y in range(y0, y0 + hauteur):
        for x in range(x0, x0 + largeur):
            a, b = image.pixel(x, y), autre.pixel(x, y)
            if any(abs(p - q) > tolerance for p, q in zip(a, b, strict=True)):
                total += 1
    return total


def luminance(pixel):
    rouge, vert, bleu = pixel
    return (2126 * rouge + 7152 * vert + 722 * bleu) // 10000


def verifier_ecran_de_connexion(image):
    encre = image.compter(HORLOGE, ENCRE, TOLERANCE_ENCRE)
    assert encre > PIXELS_ENCRE_HORLOGE_MINI, f"horloge 120 px attendue ({encre} pixels d'encre)"
    dedans = luminance(image.pixel(CARTE[0] + 40, CARTE[1] + CARTE[3] - 12))
    dehors = luminance(image.pixel(CARTE[0] - 40, CARTE[1] + CARTE[3] - 12))
    assert dedans > dehors, (
        f"carte en verre blanc plus claire que le fond ({dedans} contre {dehors})"
    )


def verifier_dock(bureau, connexion):
    surface = DOCK[2] * DOCK[3]
    changes = modifies(bureau, connexion, DOCK)
    assert changes > PART_DOCK_MODIFIEE_MINI * surface, (
        f"dock en verre attendu ({changes} pixels sur {surface})"
    )
    sous = modifies(bureau, connexion, BANDE_SOUS_DOCK)
    assert sous < PIXELS_HORS_ZONE_MAXI, f"marge de 12 px sous le dock inchangée ({sous} pixels)"
    clair = luminance(bureau.pixel(ORBE[0] + ORBE[2] // 2, ORBE[1] + 3))
    assert clair > 180, f"orbe de 42 px en verre clair attendu (luminance {clair})"


def verifier_lanceur(menu, bureau):
    surface = LANCEUR[2] * LANCEUR[3]
    changes = modifies(menu, bureau, LANCEUR)
    assert changes > PART_LANCEUR_MODIFIEE_MINI * surface, (
        f"panneau de 540 x 289 attendu ({changes} pixels sur {surface})"
    )
    dehors = modifies(menu, bureau, HORS_LANCEUR)
    assert dehors < PIXELS_HORS_ZONE_MAXI, (
        f"rien à droite ni au-dessus du lanceur ({dehors} pixels)"
    )


def attendre_arret(vm, delai):
    try:
        vm.processus.wait(timeout=delai)
    except subprocess.TimeoutExpired as erreur:
        raise AssertionError("QEMU devait quitter après « Arrêter »") from erreur


def run(vm):
    vm.attendre(r"logon: écran de connexion prêt", delai=120)
    ecran_connexion = vm.capture_stable(nom="connexion.ppm")
    assert ecran_connexion.largeur == LARGEUR and ecran_connexion.hauteur == HAUTEUR
    verifier_ecran_de_connexion(ecran_connexion)

    vm.touche("ret")
    vm.attendre(r"logon: session ouverte", delai=60)
    vm.attendre(r"shell: bureau prêt", delai=60)
    bureau = vm.capture_stable(nom="bureau.ppm")
    verifier_dock(bureau, ecran_connexion)

    vm.touche("meta_l")
    vm.capture_quand(
        lambda i: modifies(i, bureau, LANCEUR) > 0.5 * LANCEUR[2] * LANCEUR[3], nom="menu.ppm"
    )
    menu = vm.capture_stable(nom="menu.ppm")
    verifier_lanceur(menu, bureau)
    vm.touche("esc")
    ferme = vm.capture_quand(
        lambda i: modifies(i, bureau, LANCEUR) < PIXELS_MENU_FERME_MAXI, nom="menu_ferme.ppm"
    )
    reste = modifies(ferme, bureau, LANCEUR)
    assert reste < PIXELS_MENU_FERME_MAXI, f"Échap doit refermer le lanceur ({reste} pixels)"

    vm.touche("meta_l")
    vm.touche("end")
    vm.touche("ret")
    vm.touche("ret")
    attendre_arret(vm, delai=60)
