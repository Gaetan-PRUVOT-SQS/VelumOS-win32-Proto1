#!/usr/bin/env python3
import gzip
import json
import sys
from collections import defaultdict
from pathlib import Path


def charger(dossier):
    lignes = defaultdict(int)
    branches = defaultdict(int)
    for chemin in sorted(Path(dossier).glob("*.gcov.json.gz")):
        with gzip.open(chemin, "rt", encoding="utf-8") as fichier:
            donnees = json.load(fichier)
        for fichier_src in donnees["files"]:
            nom = fichier_src["file"]
            for ligne in fichier_src["lines"]:
                cle = (nom, ligne["line_number"])
                lignes[cle] += ligne["count"]
                for i, branche in enumerate(ligne.get("branches", [])):
                    branches[(*cle, i)] += branche["count"]
    return lignes, branches


def resume(lignes, branches, motifs):
    sortie = []
    for motif in motifs:
        cles = [c for c in lignes if c[0].endswith(motif)]
        if not cles:
            sortie.append((motif, 0, 0, 0, 0, []))
            continue
        vues = [c for c in cles if lignes[c] > 0]
        manquantes = sorted(c[1] for c in cles if lignes[c] == 0)
        bcles = [c for c in branches if c[0].endswith(motif)]
        bvues = [c for c in bcles if branches[c] > 0]
        sortie.append((motif, len(vues), len(cles), len(bvues), len(bcles), manquantes))
    return sortie


def main(argv):
    lignes, branches = charger(argv[0])
    total_l = total_v = total_b = total_bv = 0
    for motif, vues, cles, bvues, bcles, manquantes in resume(lignes, branches, argv[1:]):
        total_l += cles
        total_v += vues
        total_b += bcles
        total_bv += bvues
        print(f"{motif} : lignes {vues}/{cles}, branches {bvues}/{bcles}, manquantes {manquantes}")
    print(f"TOTAL : lignes {total_v}/{total_l}, branches {total_bv}/{total_b}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
