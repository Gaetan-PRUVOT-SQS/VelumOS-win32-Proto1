import statistics
import time
from pathlib import Path

from vtest import Image

NAME = "intégration : captures connexion, bureau, lanceur face à la maquette (README)"
CMDLINE = "quiet"
MEMOIRE = "256M"

LARGEUR = 1024
HAUTEUR = 768
LARGEUR_MAQUETTE = 1280
HAUTEUR_MAQUETTE = 800
PAUSE = 0.12
ICONE_BONJOUR_MAQUETTE = (69, 350)
FENETRE_BONJOUR_MAQUETTE = (330, 160, 480, 235)
DOSSIER_MAQUETTE = (
    Path(__file__).resolve().parents[2] / "ux design" / "design_handoff_velumos_glass" / "captures"
)
MAQUETTES = {
    "connexion": "01-connexion-clair.png",
    "bureau": "02-bureau-clair.png",
    "lanceur": "03-lanceur-clair.png",
}
SEUIL_ECART_CANAL = 28
TOLERANCE_FOND = 28
PART_CONTENU_MINI = 0.4
PART_VIDE_MAXI = 0.03
PAS_ECHANTILLON = 2

POINTS_FOND = (
    (0.12, 0.18),
    (0.88, 0.12),
    (0.72, 0.80),
    (0.30, 0.62),
    (0.50, 0.45),
)

ZONES = {
    "connexion": (
        ("horloge", "c", (450, 100, 380, 120)),
        ("date", "c", (450, 225, 380, 40)),
        ("carte", "c", (450, 311, 380, 279)),
        ("avatar", "c", (596, 339, 88, 88)),
        ("nom", "c", (560, 440, 160, 34)),
        ("champ mot de passe", "c", (474, 487, 332, 48)),
        ("puce changer d'utilisateur", "bl", (28, 728, 202, 44)),
        ("puce FR", "bl", (240, 728, 50, 44)),
        ("puce éteindre", "br", (1132, 728, 120, 44)),
    ),
    "bureau": (
        ("icône Ordinateur", "tl", (24, 24, 90, 81)),
        ("icône Documents", "tl", (24, 123, 90, 81)),
        ("icône Corbeille", "tl", (24, 222, 90, 81)),
        ("icône Bonjour", "tl", (24, 321, 90, 81)),
        ("orbe", "bl", (20, 739, 42, 42)),
        ("zone de notification", "br", (1056, 738, 202, 44)),
    ),
    "fenetre": (("fenêtre Bonjour", "tl", FENETRE_BONJOUR_MAQUETTE),),
    "lanceur": (
        ("panneau", "bl", (12, 195, 540, 525)),
        ("recherche", "bl", (30, 214, 504, 48)),
        ("tuiles, première rangée", "bl", (30, 305, 500, 96)),
        ("tuiles, seconde rangée", "bl", (30, 407, 500, 96)),
        ("récents", "bl", (36, 520, 490, 110)),
        ("pied", "bl", (30, 655, 504, 48)),
    ),
}

ZONES_VIDES = {
    "connexion": (
        ("fond sous l'horloge, à gauche", "c", (0, 280, 400, 200)),
        ("fond à droite de la carte", "c", (860, 300, 380, 280)),
    ),
    "bureau": (
        ("centre du bureau", "c", (420, 430, 440, 250)),
        ("haut à droite", "c", (860, 20, 380, 120)),
    ),
    "lanceur": (
        ("fond à droite du lanceur", "c", (620, 100, 600, 500)),
        ("haut du bureau", "c", (140, 20, 600, 150)),
    ),
}

EXCLUSIONS_FOND = {
    "connexion": ((430, 80, 420, 530),),
    "bureau": ((330, 160, 480, 235), (0, 0, 130, 420)),
    "lanceur": ((0, 180, 560, 560), (0, 0, 130, 420)),
}


def ancrer(ancre, rect, largeur, hauteur):
    x, y, w, h = rect
    ecart_x = largeur - LARGEUR_MAQUETTE
    ecart_y = hauteur - HAUTEUR_MAQUETTE
    if ancre == "c":
        return (x + ecart_x // 2, y, w, h)
    if ancre == "bl":
        return (x, y + ecart_y, w, h)
    if ancre == "br":
        return (x + ecart_x, y + ecart_y, w, h)
    return (x, y, w, h)


def charger_maquette(nom):
    from PIL import Image as PilImage

    brute = PilImage.open(DOSSIER_MAQUETTE / MAQUETTES[nom]).convert("RGB")
    assert brute.size == (LARGEUR_MAQUETTE, HAUTEUR_MAQUETTE), f"capture {nom} : {brute.size}"
    return Image(brute.width, brute.height, brute.tobytes())


def moyenne_bloc(image, cx, cy, demi=3):
    somme = [0, 0, 0]
    total = 0
    for y in range(max(0, cy - demi), min(image.hauteur, cy + demi + 1)):
        for x in range(max(0, cx - demi), min(image.largeur, cx + demi + 1)):
            for canal, valeur in enumerate(image.pixel(x, y)):
                somme[canal] += valeur
            total += 1
    return tuple(valeur // total for valeur in somme)


def part_contenu(image, rect):
    x0, y0, largeur, hauteur = rect
    pixels = [
        image.pixel(x, y)
        for y in range(y0, y0 + hauteur, PAS_ECHANTILLON)
        for x in range(x0, x0 + largeur, PAS_ECHANTILLON)
    ]
    fond = tuple(int(statistics.median(p[c] for p in pixels)) for c in range(3))
    different = sum(
        1 for p in pixels if max(abs(p[c] - fond[c]) for c in range(3)) > SEUIL_ECART_CANAL
    )
    return different / len(pixels)


def comparer_zones(ecarts, ecran, nom, zones, vides, image, maquette):
    for etiquette, ancre, rect in zones:
        attendu = part_contenu(maquette, rect)
        obtenu = part_contenu(image, ancrer(ancre, rect, image.largeur, image.hauteur))
        if obtenu < PART_CONTENU_MINI * attendu:
            ecarts.append(
                f"{ecran} : {etiquette} trop vide ({obtenu:.2f} de contenu, maquette {attendu:.2f})"
            )
    for etiquette, ancre, rect in vides:
        attendu = part_contenu(maquette, rect)
        obtenu = part_contenu(image, ancrer(ancre, rect, image.largeur, image.hauteur))
        borne = max(PART_VIDE_MAXI, 2 * attendu)
        if obtenu > borne:
            ecarts.append(
                f"{ecran} : {etiquette} devrait être du fond nu ({obtenu:.2f}, borne {borne:.2f})"
            )


def comparer_fond(ecarts, ecran, exclusions, image, maquette):
    for fx, fy in POINTS_FOND:
        mx, my = int(fx * LARGEUR_MAQUETTE), int(fy * HAUTEUR_MAQUETTE)
        if any(rx <= mx < rx + rw and ry <= my < ry + rh for rx, ry, rw, rh in exclusions):
            continue
        attendu = moyenne_bloc(maquette, mx, my)
        obtenu = moyenne_bloc(image, int(fx * image.largeur), int(fy * image.hauteur))
        if any(abs(a - b) > TOLERANCE_FOND for a, b in zip(attendu, obtenu, strict=True)):
            ecarts.append(f"{ecran} : fond en ({fx}, {fy}) {obtenu}, maquette {attendu}")


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
    image = vm.capture_stable(nom=f"{nom}.ppm")
    image.enregistrer_png(vm.dossier / f"{vm.nom}.{nom}.png")
    return image


def verifier_ecran(ecarts, ecran, image, cle=None):
    cle = cle or ecran
    maquette = charger_maquette(ecran)
    comparer_zones(ecarts, ecran, ecran, ZONES[cle], ZONES_VIDES.get(cle, ()), image, maquette)
    if ecran in EXCLUSIONS_FOND:
        comparer_fond(ecarts, ecran, EXCLUSIONS_FOND[ecran], image, maquette)


def run(vm):
    ecarts = []

    vm.attendre(r"logon: écran de connexion prêt", delai=120)
    time.sleep(1)
    connexion = enregistrer(vm, "connexion")
    assert (connexion.largeur, connexion.hauteur) == (LARGEUR, HAUTEUR)
    verifier_ecran(ecarts, "connexion", connexion)

    vm.touche("ret")
    vm.attendre(r"logon: session ouverte", delai=60)
    vm.attendre(r"shell: bureau prêt", delai=60)
    time.sleep(3)
    verifier_ecran(ecarts, "bureau", enregistrer(vm, "bureau"))

    cliquer(vm, *ICONE_BONJOUR_MAQUETTE, fois=2)
    vm.attendre(r"hello", delai=20)
    time.sleep(2)
    fenetre = enregistrer(vm, "fenetre-bonjour")
    comparer_zones(
        ecarts, "fenêtre", "bureau", ZONES["fenetre"], (), fenetre, charger_maquette("bureau")
    )

    vm.touche("meta_l")
    time.sleep(1)
    verifier_ecran(ecarts, "lanceur", enregistrer(vm, "lanceur"))

    assert not ecarts, "écarts à la maquette :\n  " + "\n  ".join(ecarts)
