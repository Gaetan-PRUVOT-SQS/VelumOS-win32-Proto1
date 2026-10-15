#!/usr/bin/env python3
import argparse
import concurrent.futures
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parents[3]
FICHIER = re.compile(r"^File '(.+)'$")
MESURE = re.compile(
    r"^(Lines executed|Branches executed|Taken at least once|Condition outcomes covered)"
    r":([\d.]+)% of (\d+)$"
)


def compiler(args, test, sortie):
    dossier = sortie / test.stem
    dossier.mkdir(parents=True, exist_ok=True)
    commande = [
        args.hostcc,
        *args.flags.split(),
        "--coverage",
        "-fcondition-coverage",
        str(test),
        "tests/host/harness.c",
        "tests/host/harness_eq.c",
        *args.sources.split(),
        *sorted(str(f) for f in Path("tests/host/a13").glob("fake_*.c")),
        "-o",
        str(dossier / "t"),
    ]
    resultat = subprocess.run(commande, cwd=RACINE, capture_output=True, text=True, check=False)
    if resultat.returncode:
        sys.stderr.write(resultat.stderr)
        raise SystemExit(f"compilation impossible : {test}")
    execution = subprocess.run([str(dossier / "t")], cwd=RACINE, capture_output=True, check=False)
    if execution.returncode:
        raise SystemExit(f"test en echec : {test}")
    return dossier


def fusionner(dossiers, sortie):
    courant = dossiers[0]
    for numero, suivant in enumerate(dossiers[1:]):
        cible = sortie / f"fusion{numero}"
        if cible.exists():
            shutil.rmtree(cible)
        subprocess.run(
            ["gcov-tool", "merge", str(courant), str(suivant), "-o", str(cible)],
            check=True,
            capture_output=True,
        )
        courant = cible
    for gcno in dossiers[0].glob("t-*.gcno"):
        shutil.copy(gcno, courant / gcno.name)
    return courant


def resumer(dossier, sources):
    mesures = {}
    for gcda in sorted(dossier.glob("t-*.gcda")):
        if not gcda.with_suffix(".gcno").exists():
            continue
        resultat = subprocess.run(
            ["gcov", "-b", "-c", "-n", "--conditions", str(gcda)],
            cwd=RACINE,
            env={**os.environ, "LC_ALL": "C"},
            capture_output=True,
            text=True,
            check=True,
        )
        courant = None
        for ligne in resultat.stdout.splitlines():
            trouve = FICHIER.match(ligne)
            if trouve:
                courant = trouve.group(1)
                continue
            valeur = MESURE.match(ligne)
            if (
                valeur
                and courant in sources
                and courant not in mesures.setdefault(valeur.group(1), {})
            ):
                mesures[valeur.group(1)][courant] = (float(valeur.group(2)), int(valeur.group(3)))
    return mesures


def afficher(mesures, sources):
    for nom, par_fichier in mesures.items():
        total = sum(n for _, n in par_fichier.values())
        couvert = sum(p * n / 100 for p, n in par_fichier.values())
        print(f"{nom} : {100 * couvert / total:.1f} % de {total}" if total else f"{nom} : n/a")
    print("detail des lignes par fichier :")
    for fichier in sorted(sources):
        ligne = mesures.get("Lines executed", {}).get(fichier)
        branche = mesures.get("Taken at least once", {}).get(fichier)
        texte_l = f"{ligne[0]:5.1f} % de {ligne[1]:3d}" if ligne else "n/a"
        texte_b = f"{branche[0]:5.1f} % de {branche[1]:3d}" if branche else "n/a"
        print(f"  {fichier:40s} lignes {texte_l}  branches {texte_b}")


def main():
    parseur = argparse.ArgumentParser()
    parseur.add_argument("--out", required=True)
    parseur.add_argument("--hostcc", default="gcc")
    parseur.add_argument("--tests", required=True)
    parseur.add_argument("--sources", required=True)
    parseur.add_argument("--flags", required=True)
    parseur.add_argument("--reuse", action="store_true")
    args = parseur.parse_args()
    sortie = RACINE / args.out
    tests = [Path(t) for t in args.tests.split()]
    if args.reuse:
        dossiers = [sortie / t.stem for t in tests]
    else:
        if sortie.exists():
            shutil.rmtree(sortie)
        sortie.mkdir(parents=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:
            dossiers = list(pool.map(lambda t: compiler(args, t, sortie), tests))
    fusion = fusionner(dossiers, sortie)
    sources = {s for s in args.sources.split() if s.startswith(("drivers/", "kernel/kcon/"))}
    afficher(resumer(fusion, sources), sources)


if __name__ == "__main__":
    main()
