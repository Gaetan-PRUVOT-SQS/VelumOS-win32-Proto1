#!/usr/bin/env python3
import argparse
import hashlib
import os
import sys
from pathlib import Path

COMPTE_DEFAUT = "Utilisateur"
NOM_MAX = 31
ITERATIONS_DEFAUT = 10_000
ITERATIONS_MIN = 10_000
ITERATIONS_MAX = 1_000_000
SEL_DEFAUT = 16
SEL_MIN = 8
SEL_MAX = 32
MOT_DE_PASSE_MAX = 1024
DRAPEAUX_CONNUS = 0x1
VARIABLE_REPRO = "VELUM_REPRO"


class ErreurCompte(ValueError):
    pass


def valider_nom(nom):
    octets = nom.encode("utf-8")
    if not 1 <= len(octets) <= NOM_MAX:
        raise ErreurCompte(f"nom de 1 à {NOM_MAX} octets attendu")
    if nom != nom.strip(" "):
        raise ErreurCompte("espace en début ou fin de nom refusé")
    if any(ord(c) < 0x20 or ord(c) == 0x7F or c == ":" for c in nom):
        raise ErreurCompte("caractère de contrôle ou ':' refusé dans le nom")


def valider_parametres(iterations, sel, mot_de_passe, drapeaux):
    if not ITERATIONS_MIN <= iterations <= ITERATIONS_MAX:
        raise ErreurCompte(f"itérations entre {ITERATIONS_MIN} et {ITERATIONS_MAX}")
    if not SEL_MIN <= len(sel) <= SEL_MAX:
        raise ErreurCompte(f"sel de {SEL_MIN} à {SEL_MAX} octets")
    if len(mot_de_passe) > MOT_DE_PASSE_MAX:
        raise ErreurCompte(f"mot de passe limité à {MOT_DE_PASSE_MAX} octets")
    if drapeaux & ~DRAPEAUX_CONNUS:
        raise ErreurCompte("drapeaux inconnus")


def empreinte(mot_de_passe, sel, iterations):
    return hashlib.pbkdf2_hmac("sha256", mot_de_passe, sel, iterations, dklen=32)


def ligne_compte(nom, iterations, sel, mot_de_passe, drapeaux=0):
    valider_nom(nom)
    valider_parametres(iterations, sel, mot_de_passe, drapeaux)
    hache = empreinte(mot_de_passe, sel, iterations)
    return f"{nom}:{iterations}:{sel.hex()}:{hache.hex()}:{drapeaux}\n"


def ecrire_atomiquement(chemin, texte):
    chemin = Path(chemin)
    chemin.parent.mkdir(parents=True, exist_ok=True)
    provisoire = chemin.with_name(chemin.name + ".nouveau")
    provisoire.write_text(texte, encoding="utf-8")
    provisoire.chmod(0o644)
    os.replace(provisoire, chemin)


def analyser(argv):
    parseur = argparse.ArgumentParser(description="Fabrique /system/etc/users de VelumOS")
    parseur.add_argument("--sortie", required=True)
    parseur.add_argument("--compte", default=COMPTE_DEFAUT)
    parseur.add_argument("--iterations", type=int, default=ITERATIONS_DEFAUT)
    parseur.add_argument("--sel-hex", help="sel fixe (tests, build reproductible)")
    parseur.add_argument(
        "--sel-reproductible",
        action="store_true",
        help=f"sel dérivé de SOURCE_DATE_EPOCH et du compte, mot de passe vide seulement "
        f"(actif aussi si {VARIABLE_REPRO}=1 et mot de passe vide)",
    )
    parseur.add_argument(
        "--mot-de-passe-stdin",
        action="store_true",
        help="lit le mot de passe sur l'entrée standard (vide par défaut)",
    )
    return parseur.parse_args(argv)


def sel_reproductible(compte, mot_de_passe, environnement):
    if mot_de_passe:
        raise ErreurCompte("sel reproductible refusé avec un mot de passe : fournir --sel-hex")
    epoque = environnement.get("SOURCE_DATE_EPOCH", "0")
    if not epoque.isascii() or not epoque.isdigit():
        raise ErreurCompte("SOURCE_DATE_EPOCH invalide")
    graine = f"velumos-users-sel\0{epoque}\0{compte}".encode()
    return hashlib.sha256(graine).digest()[:SEL_DEFAUT]


def lire_sel(sel_hex, reproductible=False, compte="", mot_de_passe=b"", environnement=None):
    if sel_hex is not None:
        try:
            return bytes.fromhex(sel_hex)
        except ValueError as erreur:
            raise ErreurCompte("sel hexadécimal invalide") from erreur
    if reproductible:
        return sel_reproductible(compte, mot_de_passe, environnement or {})
    return os.urandom(SEL_DEFAUT)


def lire_mot_de_passe(depuis_stdin):
    if not depuis_stdin:
        return b""
    return sys.stdin.buffer.read().rstrip(b"\r\n")


def main(argv):
    args = analyser(argv)
    try:
        mot_de_passe = lire_mot_de_passe(args.mot_de_passe_stdin)
        demande_par_env = os.environ.get(VARIABLE_REPRO) == "1" and not mot_de_passe
        reproductible = args.sel_reproductible or demande_par_env
        sel = lire_sel(args.sel_hex, reproductible, args.compte, mot_de_passe, os.environ)
        ligne = ligne_compte(args.compte, args.iterations, sel, mot_de_passe)
    except ErreurCompte as erreur:
        print(f"mkuser: {erreur}", file=sys.stderr)
        return 2
    ecrire_atomiquement(args.sortie, ligne)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
