#!/usr/bin/env python3
"""Rapport de contraste WCAG d'un texte sur son fond, mesuré dans une capture.

python3 tools/contraste.py CAPTURE.png NOM:X0,Y0,X1,Y1 [NOM:X0,Y0,X1,Y1 ...]

Dans chaque zone : le fond est la médiane des pixels du pourtour (avec une marge autour du texte),
le texte est le pixel le plus éloigné du fond en luminance relative.
"""

import sys

from PIL import Image

SEUIL_TEXTE_COURANT = 4.5


def luminance(couleur):
    def canal(valeur):
        c = valeur / 255
        return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4

    rouge, vert, bleu = (canal(v) for v in couleur)
    return 0.2126 * rouge + 0.7152 * vert + 0.0722 * bleu


def rapport(clair, sombre):
    haut, bas = sorted((luminance(clair), luminance(sombre)), reverse=True)
    return (haut + 0.05) / (bas + 0.05)


def mediane(valeurs):
    ordre = sorted(valeurs)
    return ordre[len(ordre) // 2]


def mesurer(image, zone):
    x0, y0, x1, y1 = zone
    pixels = [image.getpixel((x, y)) for y in range(y0, y1) for x in range(x0, x1)]
    bord = [image.getpixel((x, y)) for x in range(x0, x1) for y in (y0, y1 - 1)]
    bord += [image.getpixel((x, y)) for y in range(y0, y1) for x in (x0, x1 - 1)]
    fond = tuple(mediane([p[c] for p in bord]) for c in range(3))
    texte = max(pixels, key=lambda p: abs(luminance(p) - luminance(fond)))
    return fond, texte, rapport(fond, texte)


def lire_zone(argument):
    nom, _, coordonnees = argument.partition(":")
    x0, y0, x1, y1 = (int(v) for v in coordonnees.split(","))
    return nom, (x0, y0, x1, y1)


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    image = Image.open(argv[0]).convert("RGB")
    echecs = 0
    for argument in argv[1:]:
        nom, zone = lire_zone(argument)
        fond, texte, valeur = mesurer(image, zone)
        verdict = "OK" if valeur >= SEUIL_TEXTE_COURANT else "SOUS LE SEUIL"
        echecs += valeur < SEUIL_TEXTE_COURANT
        print(f"{nom:34} fond {fond} texte {texte} contraste {valeur:5.2f} {verdict}")
    return 1 if echecs else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
