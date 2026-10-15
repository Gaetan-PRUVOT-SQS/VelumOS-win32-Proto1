#!/usr/bin/env python3
"""Contrôle statique minimal : fonctions C dangereuses, secrets, idiomes risqués."""

import argparse
import fnmatch
import os
import re
import subprocess
import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent
REGLES = {
    ".c": [r"\b(strcpy|strcat|sprintf|vsprintf|gets)\s*\("],
    ".h": [r"\b(strcpy|strcat|sprintf|vsprintf|gets)\s*\("],
    ".py": [r"\bos\.system\s*\(", r"shell\s*=\s*True", r"\beval\s*\(", r"\bexec\s*\("],
    ".sh": [r"\beval\s", r"curl[^|\n]*\|\s*(ba)?sh"],
}
SECRETS = [
    r"ghp_[A-Za-z0-9]{30,}",
    r"github_pat_[A-Za-z0-9_]{30,}",
    r"AKIA[0-9A-Z]{16}",
    r"-----BEGIN [A-Z ]*PRIVATE KEY-----",
]
EXCLUS = ("third_party/", "build/", "LICENSES/", "ux design/")
GITIGNORE_DE_BASE = ("build/", "__pycache__/", "*.pyc", "*.log", ".pytest_cache/", ".ruff_cache/")


def est_depot_git(racine):
    try:
        sortie = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=racine,
            capture_output=True,
            check=True,
        ).stdout.decode()
    except (OSError, subprocess.CalledProcessError):
        return False
    return Path(sortie.strip()).resolve() == racine.resolve()


def fichiers_git(racine):
    sortie = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
        cwd=racine,
        capture_output=True,
        check=True,
    ).stdout.decode()
    return [f for f in sortie.split("\0") if f]


def motifs_gitignore(racine):
    motifs = list(GITIGNORE_DE_BASE)
    fichier = racine / ".gitignore"
    if fichier.is_file():
        for ligne in fichier.read_text(errors="replace").splitlines():
            ligne = ligne.strip()
            if ligne and not ligne.startswith(("#", "!")):
                motifs.append(ligne.lstrip("/"))
    return motifs


def dossier_ignore(nom, motifs):
    return any(m.endswith("/") and fnmatch.fnmatch(nom, m[:-1]) for m in motifs)


def fichier_ignore(nom, motifs):
    return any(not m.endswith("/") and fnmatch.fnmatch(nom, m) for m in motifs)


def fichiers_disque(racine):
    motifs = motifs_gitignore(racine)
    trouves = []
    for dossier, sous_dossiers, noms in os.walk(racine):
        sous_dossiers[:] = sorted(d for d in sous_dossiers if not dossier_ignore(d, motifs))
        relatif = Path(dossier).relative_to(racine)
        for nom in noms:
            if not fichier_ignore(nom, motifs):
                trouves.append((relatif / nom).as_posix())
    return trouves


def fichiers(racine=RACINE):
    racine = Path(racine)
    try:
        liste = fichiers_git(racine) if est_depot_git(racine) else fichiers_disque(racine)
    except (OSError, subprocess.CalledProcessError):
        liste = fichiers_disque(racine)
    return sorted(f for f in liste if not f.startswith(EXCLUS))


def lire_exceptions(racine):
    fichier = Path(racine) / ".cyber-ignore"
    if not fichier.is_file():
        return []
    exceptions = []
    texte = fichier.read_text(encoding="utf-8-sig", errors="replace")
    for ligne in texte.splitlines():
        ligne = re.sub(r"[ \t]*#.*", "", ligne).strip()
        ligne = ligne.removeprefix("./")
        if ligne:
            exceptions.append(ligne)
    return exceptions


def est_exception(chemin, numero, exceptions):
    for e in exceptions:
        if e in (chemin, f"{chemin}:{numero}"):
            return True
        if e.endswith("/") and chemin.startswith(e):
            return True
    return False


def analyser(racine, chemin, exceptions=()):
    texte = (Path(racine) / chemin).read_text(errors="replace")
    motifs = REGLES.get(Path(chemin).suffix, []) + SECRETS
    trouves = []
    for numero, ligne in enumerate(texte.splitlines(), start=1):
        if any(re.search(m, ligne) for m in motifs):
            if not est_exception(chemin, numero, exceptions):
                trouves.append(f"{chemin}:{numero}: {ligne.strip()[:100]}")
    return trouves


def main(argv=None):
    parseur = argparse.ArgumentParser(description=__doc__)
    parseur.add_argument("--racine", type=Path, default=RACINE)
    racine = parseur.parse_args(argv).racine.resolve()
    exceptions = lire_exceptions(racine)
    trouves = []
    for chemin in fichiers(racine):
        if (racine / chemin).is_file():
            trouves += analyser(racine, chemin, exceptions)
    if trouves:
        print("\n".join(trouves))
    print(f"scan sécurité : {len(trouves)} alerte(s)", file=sys.stderr)
    return 1 if trouves else 0


if __name__ == "__main__":
    sys.exit(main())
