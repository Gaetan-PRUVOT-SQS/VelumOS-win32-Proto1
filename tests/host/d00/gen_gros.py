"""Description TOML d'un APK de plus de 2 Mio, pour le condensat par blocs de la signature v2."""

import argparse
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent.parent
MODELE = RACINE / "user" / "apk" / "bonjour" / "bonjour.toml"
CHAINES = 80
LONGUEUR = 30000


def chaine(rang):
    motif = f"{rang:04d}" + "abcdefghijklmnopqrstuvwxyz"
    return (motif * (LONGUEUR // len(motif) + 1))[:LONGUEUR]


def main():
    analyseur = argparse.ArgumentParser(description=__doc__)
    analyseur.add_argument("--sortie", required=True)
    sortie = Path(analyseur.parse_args().sortie)
    lignes = "".join(f'remplissage{rang:02d} = "{chaine(rang)}"\n' for rang in range(CHAINES))
    texte = MODELE.read_text(encoding="utf-8").replace("[chaines.fr]", lignes + "\n[chaines.fr]")
    sortie.parent.mkdir(parents=True, exist_ok=True)
    sortie.write_text(texte, encoding="utf-8")


if __name__ == "__main__":
    main()
