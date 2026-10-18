#!/usr/bin/env python3
"""Lance des binaires de test hôte et rapporte chaque échec."""

import subprocess
import sys


def lancer(binaires):
    echecs = []
    for chemin in binaires:
        print(f"host {chemin}", flush=True)
        try:
            resultat = subprocess.run([chemin], check=False)
        except OSError as exc:
            detail = exc.strerror or exc.__class__.__name__
            print(f"host {chemin} : {detail}", flush=True)
            echecs.append(chemin)
            continue
        if resultat.returncode != 0:
            echecs.append(chemin)
    if not echecs:
        return 0
    n = len(echecs)
    mot = "échec" if n == 1 else "échecs"
    print(f"suite hôte : {n} {mot}", flush=True)
    for chemin in echecs:
        print(f"  {chemin}", flush=True)
    return 1


def main(argv):
    return lancer(argv[1:])


if __name__ == "__main__":
    sys.exit(main(sys.argv))
