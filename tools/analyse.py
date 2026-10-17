#!/usr/bin/env python3
import argparse
import json
import re
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path

OUTILS = ("fanalyzer", "fanalyzer-profond", "cppcheck-normal", "cppcheck-exhaustive", "clang-tidy")
PORTEES = ("code", "tests", "tout")
VERSION_BASE = 1

MOTIF_GCC = re.compile(
    r"^(?P<fichier>[^\s:][^:]*):(?P<ligne>\d+):(?P<colonne>\d+): "
    r"(?P<gravite>warning|error): (?P<message>.*)$"
)
MOTIF_OPTION_GCC = re.compile(r"\[(-W[\w+-]+)\]\s*$")
MOTIF_TIDY = re.compile(
    r"^(?P<fichier>[^\s:][^:]*):(?P<ligne>\d+):(?P<colonne>\d+): "
    r"(?P<gravite>warning|error): (?P<message>.*) \[(?P<check>[\w.,-]+)\]$"
)

ERREURS_OUTIL_CPPCHECK = {
    "syntaxError",
    "internalError",
    "cppcheckError",
    "preprocessorErrorDirective",
}
ERREURS_OUTIL_TIDY = {"clang-diagnostic-error"}


@dataclass(frozen=True)
class Constat:
    outil: str
    fichier: str
    ligne: int
    check: str
    message: str

    def cle(self):
        return (self.outil, self.fichier, self.ligne, self.check)


def normaliser_chemin(chemin, racine=None):
    texte = chemin.strip()
    if racine is not None:
        prefixe = str(racine).rstrip("/") + "/"
        if texte.startswith(prefixe):
            texte = texte[len(prefixe) :]
    while texte.startswith("./"):
        texte = texte[2:]
    return texte


def analyser_gcc(texte, outil="fanalyzer", racine=None):
    constats, erreurs = [], []
    for ligne in texte.splitlines():
        trouve = MOTIF_GCC.match(ligne)
        if not trouve:
            continue
        message = trouve["message"]
        option = MOTIF_OPTION_GCC.search(message)
        check = option.group(1) if option else "gcc-sans-option"
        constat = Constat(
            outil,
            normaliser_chemin(trouve["fichier"], racine),
            int(trouve["ligne"]),
            check,
            message,
        )
        (erreurs if trouve["gravite"] == "error" else constats).append(constat)
    return constats, erreurs


def analyser_cppcheck(texte, outil, racine=None):
    constats, erreurs = [], []
    for ligne in texte.splitlines():
        morceaux = ligne.split("|", 4)
        if len(morceaux) != 5 or not morceaux[1].isdigit():
            continue
        fichier, numero, _gravite, identifiant, message = morceaux
        constat = Constat(
            outil, normaliser_chemin(fichier, racine), int(numero), identifiant, message
        )
        (erreurs if identifiant in ERREURS_OUTIL_CPPCHECK else constats).append(constat)
    return constats, erreurs


def analyser_tidy(texte, racine=None):
    constats, erreurs = [], []
    for ligne in texte.splitlines():
        trouve = MOTIF_TIDY.match(ligne)
        if not trouve:
            continue
        for check in trouve["check"].split(","):
            constat = Constat(
                "clang-tidy",
                normaliser_chemin(trouve["fichier"], racine),
                int(trouve["ligne"]),
                check,
                trouve["message"],
            )
            if check in ERREURS_OUTIL_TIDY:
                erreurs.append(constat)
            else:
                constats.append(constat)
    return constats, erreurs


def analyser_texte(outil, texte, racine=None):
    if outil.startswith("fanalyzer"):
        return analyser_gcc(texte, outil, racine)
    if outil.startswith("cppcheck-"):
        return analyser_cppcheck(texte, outil, racine)
    if outil == "clang-tidy":
        return analyser_tidy(texte, racine)
    raise ValueError(f"outil inconnu : {outil}")


def dedoublonner(constats):
    uniques = {}
    for constat in constats:
        uniques.setdefault(constat.cle(), constat)
    return sorted(uniques.values(), key=lambda c: (c.fichier, c.ligne, c.check))


def repertoire(fichier):
    parties = Path(fichier).parts[:-1]
    if not parties:
        return "."
    profondeur = 3 if parties[0] == "tests" else 2
    return "/".join(parties[:profondeur])


def est_dans_portee(repertoire_cle, portee):
    if portee == "tout":
        return True
    est_test = repertoire_cle == "tests" or repertoire_cle.startswith("tests/")
    return est_test if portee == "tests" else not est_test


def compter(constats):
    return dict(sorted(Counter(repertoire(c.fichier) for c in constats).items()))


def comparer(base_outil, courant):
    hausses, baisses = {}, {}
    for cle in set(base_outil) | set(courant):
        ancien, nouveau = base_outil.get(cle, 0), courant.get(cle, 0)
        if nouveau > ancien:
            hausses[cle] = (ancien, nouveau)
        elif nouveau < ancien:
            baisses[cle] = (ancien, nouveau)
    return dict(sorted(hausses.items())), dict(sorted(baisses.items()))


def mettre_a_jour(base, comptes, portee):
    resultat = {"version": VERSION_BASE, "outils": {}}
    resultat["outils"].update((base or {}).get("outils", {}))
    for outil, compte in comptes.items():
        existant = resultat["outils"].get(outil, {})
        conserve = {k: v for k, v in existant.items() if not est_dans_portee(k, portee)}
        conserve.update({k: v for k, v in compte.items() if est_dans_portee(k, portee)})
        resultat["outils"][outil] = dict(sorted(conserve.items()))
    resultat["outils"] = dict(sorted(resultat["outils"].items()))
    return resultat


def lire_base(chemin):
    chemin = Path(chemin)
    if not chemin.is_file():
        return None
    donnees = json.loads(chemin.read_text(encoding="utf-8"))
    if donnees.get("version") != VERSION_BASE:
        raise ValueError(f"version de base non gérée : {donnees.get('version')}")
    return donnees


def ecrire_base(chemin, base):
    Path(chemin).write_text(json.dumps(base, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def fichiers_bruts(sortie, outil):
    dossier = Path(sortie) / "raw" / outil
    return sorted(dossier.rglob("*.c.txt")) if dossier.is_dir() else []


def collecter(sortie, outil, racine=None):
    constats, erreurs = [], []
    fichiers = fichiers_bruts(sortie, outil)
    for chemin in fichiers:
        texte = chemin.read_text(encoding="utf-8", errors="replace")
        trouves, en_erreur = analyser_texte(outil, texte, racine)
        constats.extend(trouves)
        erreurs.extend(en_erreur)
    return dedoublonner(constats), dedoublonner(erreurs), len(fichiers)


def filtrer_portee(constats, portee):
    return [c for c in constats if est_dans_portee(repertoire(c.fichier), portee)]


def ecrire_rapport(sortie, outil, constats, erreurs):
    dossier = Path(sortie)
    dossier.mkdir(parents=True, exist_ok=True)
    lignes = [f"{c.fichier}:{c.ligne}: [{c.check}] {c.message}" for c in constats]
    lignes += [f"{c.fichier}:{c.ligne}: ERREUR-OUTIL [{c.check}] {c.message}" for c in erreurs]
    (dossier / f"rapport-{outil}.txt").write_text("\n".join(lignes) + "\n", encoding="utf-8")
    par_check = Counter(c.check for c in constats)
    detail = {"total": len(constats), "par_check": dict(sorted(par_check.items()))}
    (dossier / f"resume-{outil}.json").write_text(
        json.dumps(detail, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )


def formater_ecarts(titre, ecarts):
    lignes = [titre]
    lignes += [f"  {cle} : {ancien} -> {nouveau}" for cle, (ancien, nouveau) in ecarts.items()]
    return "\n".join(lignes)


def construire_parseur():
    parseur = argparse.ArgumentParser(
        description="Compare les constats d'analyse statique à la base"
    )
    parseur.add_argument("--racine", default=".")
    parseur.add_argument("--sortie", default="build/analyse")
    parseur.add_argument("--base", default="tools/analyse.baseline")
    parseur.add_argument(
        "--outils", nargs="+", default=["fanalyzer", "cppcheck-normal", "clang-tidy"]
    )
    parseur.add_argument("--portee", choices=PORTEES, default="code")
    parseur.add_argument("--update-baseline", action="store_true")
    return parseur


def main(argv=None):
    options = construire_parseur().parse_args(argv)
    for outil in options.outils:
        if outil not in OUTILS:
            print(f"outil inconnu : {outil}", file=sys.stderr)
            return 2
    racine = Path(options.racine).resolve()
    sortie = (
        (racine / options.sortie)
        if not Path(options.sortie).is_absolute()
        else Path(options.sortie)
    )
    comptes, erreurs_outil = {}, []
    for outil in options.outils:
        constats, erreurs, analyses = collecter(sortie, outil, racine)
        if analyses == 0:
            print(f"{outil} : aucun résultat sous {sortie}/raw/{outil}", file=sys.stderr)
            return 2
        constats = filtrer_portee(constats, options.portee)
        erreurs = filtrer_portee(erreurs, options.portee)
        ecrire_rapport(sortie, outil, constats, erreurs)
        comptes[outil] = compter(constats)
        erreurs_outil.extend(erreurs)
        print(f"{outil} : {len(constats)} constats, {analyses} fichiers analysés")
    if erreurs_outil:
        for erreur in erreurs_outil[:20]:
            print(
                f"erreur d'outil : {erreur.fichier}:{erreur.ligne} {erreur.message}",
                file=sys.stderr,
            )
        print(f"{len(erreurs_outil)} erreurs d'outil : la base n'est pas fiable", file=sys.stderr)
        return 2
    chemin_base = racine / options.base
    base = lire_base(chemin_base)
    if options.update_baseline:
        ecrire_base(chemin_base, mettre_a_jour(base, comptes, options.portee))
        print(f"base réécrite : {options.base}")
        return 0
    return verifier(base, comptes, options)


def verifier(base, comptes, options):
    if base is None:
        print(f"base absente : {options.base} (lancer avec --update-baseline)", file=sys.stderr)
        return 1
    echec = False
    for outil, compte in comptes.items():
        attendu = base["outils"].get(outil)
        if attendu is None:
            print(f"{outil} : pas de base (lancer avec --update-baseline)", file=sys.stderr)
            echec = True
            continue
        portee_base = {k: v for k, v in attendu.items() if est_dans_portee(k, options.portee)}
        hausses, baisses = comparer(portee_base, compte)
        if hausses:
            print(
                formater_ecarts(f"{outil} : hausse par rapport à la base", hausses), file=sys.stderr
            )
            echec = True
        if baisses:
            print(formater_ecarts(f"{outil} : baisse (mettre la base à jour)", baisses))
    if echec:
        return 1
    print("analyse statique : aucun nombre n'augmente")
    return 0


if __name__ == "__main__":
    sys.exit(main())
