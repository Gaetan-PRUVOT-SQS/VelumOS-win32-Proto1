#!/usr/bin/env python3
"""Vérifie que le Makefile garde les drapeaux noyau attendus."""

import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent

COMMUN = (
    "-std=gnu11",
    "-ffreestanding",
    "-fno-pic",
    "-fno-pie",
    "-mcmodel=kernel",
    "-mno-red-zone",
    "-mgeneral-regs-only",
    "-fno-omit-frame-pointer",
    "-fno-strict-aliasing",
    "-fwrapv",
    "-fno-common",
    "-fno-delete-null-pointer-checks",
    "-fstack-protector-strong",
    "-mstack-protector-guard=global",
    "-ftrivial-auto-var-init=zero",
    "-fzero-call-used-regs=used-gpr",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wvla",
    "-Wframe-larger-than=2048",
    "-fstack-clash-protection",
)
DEBUG = (
    "-O1",
    "-fsanitize=undefined",
    "-fsanitize-undefined-trap-on-error",
    "-DVELUM_DEBUG",
)
RELEASE = (
    "-O2",
    "-DNDEBUG",
)


def lignes_jointes(texte):
    sortie = []
    tampon = None
    for brute in texte.splitlines():
        morceau = brute.strip() if tampon is not None else brute.rstrip()
        tampon = morceau if tampon is None else f"{tampon} {morceau}"
        if tampon.endswith("\\"):
            tampon = tampon[:-1].rstrip()
            continue
        sortie.append(tampon)
        tampon = None
    if tampon is not None:
        sortie.append(tampon)
    return sortie


def valeur(lignes, nom):
    for ligne in lignes:
        if ligne.startswith(f"{nom} :=") or ligne.startswith(f"{nom}:="):
            return ligne.split(":=", 1)[1].split()
    return None


def _exiger(manques, nom, attendus, trouves):
    if trouves is None:
        manques.append(f"{nom} absent")
        return
    for drapeau in attendus:
        if drapeau not in trouves:
            manques.append(f"{nom} sans {drapeau}")


def verifier(texte):
    lignes = lignes_jointes(texte)
    manques = []
    _exiger(manques, "KCFLAGS_COMMON", COMMUN, valeur(lignes, "KCFLAGS_COMMON"))
    _exiger(manques, "KCFLAGS_debug", DEBUG, valeur(lignes, "KCFLAGS_debug"))
    _exiger(manques, "KCFLAGS_release", RELEASE, valeur(lignes, "KCFLAGS_release"))
    composition = None
    for ligne in lignes:
        if ligne.startswith("KCFLAGS :=") or ligne.startswith("KCFLAGS:="):
            composition = ligne.split(":=", 1)[1]
            break
    if composition is None:
        manques.append("KCFLAGS absent")
    else:
        if "$(KCFLAGS_COMMON)" not in composition:
            manques.append("KCFLAGS sans KCFLAGS_COMMON")
        if "$(KCFLAGS_$(PROFIL))" not in composition:
            manques.append("KCFLAGS sans le profil")
    return manques


def main(argv):
    del argv
    manques = verifier((RACINE / "Makefile").read_text(encoding="utf-8"))
    for ligne in manques:
        print(ligne, file=sys.stderr)
    return 1 if manques else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
