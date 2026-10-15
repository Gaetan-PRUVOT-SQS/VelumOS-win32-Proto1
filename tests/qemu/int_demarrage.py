import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from int_captures import (  # noqa: E402
    HAUTEUR,
    LARGEUR,
    PART_CONTENU_MINI,
    PART_VIDE_MAXI,
    charger_maquette,
    comparer_fond,
    part_contenu,
)

NAME = "intégration : écran de démarrage sur le fond voile (maquette 01, README §5)"
CMDLINE = "quiet init=/aucun"
MEMOIRE = "256M"
DEMI_LARGEUR_CARTE = 190
DEMI_HAUTEUR_CARTE = 90
ASYMETRIE_MAXI = 0.15


def run(vm):
    vm.attendre(r"VelumOS boot ok", delai=60)
    time.sleep(0.5)
    image = vm.capture_stable(nom="demarrage.ppm")
    image.enregistrer_png(vm.dossier / f"{vm.nom}.demarrage.png")
    assert (image.largeur, image.hauteur) == (LARGEUR, HAUTEUR)
    ecarts = []
    cx, cy = LARGEUR // 2, HAUTEUR // 2
    gauche = part_contenu(
        image,
        (
            cx - DEMI_LARGEUR_CARTE,
            cy - DEMI_HAUTEUR_CARTE,
            DEMI_LARGEUR_CARTE,
            2 * DEMI_HAUTEUR_CARTE,
        ),
    )
    droite = part_contenu(
        image, (cx, cy - DEMI_HAUTEUR_CARTE, DEMI_LARGEUR_CARTE, 2 * DEMI_HAUTEUR_CARTE)
    )
    if max(gauche, droite) < PART_CONTENU_MINI * 0.25:
        ecarts.append(f"carte de démarrage absente au centre ({gauche:.2f}, {droite:.2f})")
    elif abs(gauche - droite) > ASYMETRIE_MAXI:
        ecarts.append(f"carte non centrée en x (gauche {gauche:.2f}, droite {droite:.2f})")
    haut = part_contenu(
        image, (cx - DEMI_LARGEUR_CARTE, 0, 2 * DEMI_LARGEUR_CARTE, cy - 2 * DEMI_HAUTEUR_CARTE)
    )
    if haut > PART_VIDE_MAXI:
        ecarts.append(f"fond nu attendu au-dessus de la carte ({haut:.2f})")
    comparer_fond(ecarts, "démarrage", ((430, 80, 420, 530),), image, charger_maquette("connexion"))
    assert not ecarts, "écarts à la maquette :\n  " + "\n  ".join(ecarts)
