#!/usr/bin/env python3
"""Matrice de couverture fonctionnelle : vérifie les références, calcule les taux, écrit les rapports.

python3 tools/matrice_couverture.py            écrit tests/MATRICE.md et tests/MANUEL.md
python3 tools/matrice_couverture.py --check    échoue si les fichiers écrits ne sont pas à jour
python3 tools/matrice_couverture.py --orphelins  liste les fichiers de tests qu'aucune fonctionnalité ne cite
"""

import re
import sys
import tomllib
from collections import Counter, OrderedDict
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent
SOURCE = RACINE / "tests" / "matrice.toml"
MATRICE = RACINE / "tests" / "MATRICE.md"
MANUEL = RACINE / "tests" / "MANUEL.md"
LIBELLES = {"A": "automatisé", "AM": "automatisé + complément manuel", "M": "manuel"}
STATUTS = ("Réussi", "Échec", "Partiel", "Bloqué", "A exécuter")
CHAMPS_EXECUTION = ("date", "moyen", "constat")
CHAMPS_TRACABILITE = ("exigence", "technique", "element")


class Erreur(Exception):
    pass


def charger():
    with SOURCE.open("rb") as flux:
        donnees = tomllib.load(flux)
    fonctions, manuels = donnees["fonction"], donnees["manuel"]
    ids = [f["id"] for f in fonctions]
    if len(ids) != len(set(ids)):
        raise Erreur("identifiants de fonctionnalité en double")
    connus = {m["id"] for m in manuels}
    verifier_manuels(manuels)
    for f in fonctions:
        if f["mode"] not in LIBELLES:
            raise Erreur(f"{f['id']} : mode inconnu {f['mode']}")
        if f["mode"] in ("A", "AM") and not f.get("tests"):
            raise Erreur(f"{f['id']} : mode {f['mode']} sans test")
        if f["mode"] in ("AM", "M") and not f.get("manuels"):
            raise Erreur(f"{f['id']} : mode {f['mode']} sans test manuel")
        for chemin in f.get("tests", []):
            if not (RACINE / chemin).is_file():
                raise Erreur(f"{f['id']} : test introuvable {chemin}")
        for ref in f.get("manuels", []):
            if ref not in connus:
                raise Erreur(f"{f['id']} : test manuel inconnu {ref}")
        verifier_tracabilite(f)
    citees = {r for f in fonctions for r in f.get("manuels", [])}
    orphelins = sorted(connus - citees)
    if orphelins:
        raise Erreur(f"tests manuels sans fonctionnalité : {orphelins}")
    return fonctions, manuels


def verifier_tracabilite(f):
    champs = [c for c in CHAMPS_TRACABILITE if f.get(c)]
    if champs and len(champs) != len(CHAMPS_TRACABILITE):
        manquants = [c for c in CHAMPS_TRACABILITE if c not in champs]
        raise Erreur(f"{f['id']} : traçabilité incomplète, manque {manquants}")
    if not f.get("cas"):
        return
    sources = "\n".join((RACINE / t).read_text(errors="replace") for t in f.get("tests", []))
    for nom in f["cas"]:
        if nom not in sources:
            raise Erreur(f"{f['id']} : cas introuvable dans les tests cités : {nom!r}")


def verifier_manuels(manuels):
    for m in manuels:
        if m["statut"] not in STATUTS:
            raise Erreur(f"{m['id']} : statut inconnu {m['statut']}")
        if m["statut"] == "A exécuter":
            continue
        for champ in CHAMPS_EXECUTION:
            if not m.get(champ):
                raise Erreur(f"{m['id']} : statut {m['statut']} sans {champ}")
        if m["statut"] == "Échec" and not m.get("incidents"):
            raise Erreur(f"{m['id']} : échec sans incident")


def pourcent(n, total):
    return f"{100 * n / total:.1f}".replace(".", ",") + " %"


def inventaire():
    cas_hote = suites = 0
    for chemin in (RACINE / "tests" / "host").rglob("test_*.c"):
        suites += 1
        cas_hote += len(re.findall(r"\bh_run\(", chemin.read_text(errors="replace")))
    scenarios = len(list((RACINE / "tests" / "qemu").glob("*.py")))
    pytests = 0
    for chemin in list((RACINE / "tests").rglob("test_*.py")):
        pytests += len(re.findall(r"^\s*def test_", chemin.read_text(), re.M))
    return {"suites": suites, "cas_hote": cas_hote, "scenarios": scenarios, "pytests": pytests}


def lien(chemin):
    nom = Path(chemin).name
    cible = (
        Path("..") / chemin if not chemin.startswith("tests/") else Path(chemin[len("tests/") :])
    )
    return f"[{nom}]({cible.as_posix()})"


def resume(fonctions, manuels):
    n = len(fonctions)
    modes = Counter(f["mode"] for f in fonctions)
    inv = inventaire()
    auto_cas = inv["cas_hote"] + inv["scenarios"] + inv["pytests"]
    manuel_cas = len(manuels)
    statuts = Counter(m["statut"] for m in manuels)
    joues = sum(statuts[x] for x in ("Réussi", "Échec", "Partiel"))
    detail = ", ".join(
        f"{statuts[x]} {x.lower()}" for x in ("Réussi", "Échec", "Partiel") if statuts[x]
    )
    lignes = [
        "| Mesure | Valeur |",
        "|--------|--------|",
        f"| Fonctionnalités vérifiables recensées | {n} |",
        f"| Couverture automatisée (A + AM) | **{pourcent(modes['A'] + modes['AM'], n)}** ({modes['A'] + modes['AM']} sur {n}) |",
        f"| dont automatisée seule (A) | {pourcent(modes['A'], n)} ({modes['A']}) |",
        f"| dont automatisée avec complément manuel (AM) | {pourcent(modes['AM'], n)} ({modes['AM']}) |",
        f"| Couverture manuelle seule (M) | **{pourcent(modes['M'], n)}** ({modes['M']} sur {n}) |",
        f"| Fonctionnalités qui demandent au moins un test manuel (AM + M) | {pourcent(modes['AM'] + modes['M'], n)} ({modes['AM'] + modes['M']}) |",
        f"| Cas de test automatisés (cas unitaires, scénarios QEMU, tests Python) | {auto_cas} |",
        f"| Cas de test manuels | {manuel_cas} |",
        f"| Cas manuels joués | {joues} sur {manuel_cas} ({detail}) |",
        f"| Taux d'automatisation par nombre de cas | {pourcent(auto_cas, auto_cas + manuel_cas)} |",
    ]
    return "\n".join(lignes), inv


def par_sous_systeme(fonctions):
    groupes = OrderedDict()
    for f in fonctions:
        groupes.setdefault(f["sous_systeme"], []).append(f)
    lignes = [
        "| Sous-système | Fonctionnalités | A | AM | M | Automatisée (A + AM) |",
        "|---|---|---|---|---|---|",
    ]
    for nom, liste in groupes.items():
        c = Counter(f["mode"] for f in liste)
        lignes.append(
            f"| {nom} | {len(liste)} | {c['A']} | {c['AM']} | {c['M']} | {pourcent(c['A'] + c['AM'], len(liste))} |"
        )
    return "\n".join(lignes)


def tableau(fonctions):
    lignes = [
        "| ID | Sous-système | Fonctionnalité | Mode | Tests automatisés | Tests manuels |",
        "|---|---|---|---|---|---|",
    ]
    for f in fonctions:
        tests = "<br>".join(lien(t) for t in f.get("tests", [])) or "aucun"
        manuels = ", ".join(f.get("manuels", [])) or "aucun"
        note = f" ({f['note']})" if f.get("note") else ""
        lignes.append(
            f"| {f['id']} | {f['sous_systeme']} | {f['titre']}{note} | {f['mode']} | {tests} | {manuels} |"
        )
    return "\n".join(lignes)


def tracabilite(fonctions):
    lignes = [
        "| ID | Exigence | Technique (29119-4) | Élément de couverture | Cas nommés |",
        "|---|---|---|---|---|",
    ]
    for f in fonctions:
        if not f.get("technique"):
            continue
        cas = "<br>".join(f"`{c}`" for c in f.get("cas", [])) or "voir fichiers"
        lignes.append(
            f"| {f['id']} | {f['exigence']} | {f['technique']} | {f['element']} | {cas} |"
        )
    return "\n".join(lignes)


def tests_orphelins(fonctions):
    cites = {t for f in fonctions for t in f.get("tests", [])}
    racine_tests = RACINE / "tests"
    trouves = set()
    for chemin in racine_tests.rglob("*"):
        relatif = chemin.relative_to(RACINE).as_posix()
        if not chemin.is_file() or "__pycache__" in relatif:
            continue
        hote = relatif.startswith("tests/host/") and chemin.name.startswith("test_")
        hote = hote and chemin.suffix in (".c", ".py")
        qemu = relatif.startswith("tests/qemu/") and chemin.suffix == ".py"
        outils = relatif.startswith("tests/tools/") and chemin.name.startswith("test_")
        if hote or qemu or outils or relatif.startswith("tests/abi/"):
            trouves.add(relatif)
    return sorted(trouves - cites)


def ecrire_matrice(fonctions, manuels):
    chiffres, inv = resume(fonctions, manuels)
    return f"""# Matrice de couverture fonctionnelle

Fichier généré par `tools/matrice_couverture.py` à partir de [`matrice.toml`](matrice.toml). Ne pas l'éditer à la main.
La commande vérifie aussi que chaque test cité existe.

## Définitions

La matrice recense les fonctionnalités vérifiables de la première tranche de VelumOS, regroupées par sous-système
(voir le README pour la table a01 à a20). Chaque fonctionnalité a un mode de vérification :

| Mode | Sens |
|------|------|
| A | vérifiée par des tests automatisés, avec un oracle automatique (valeur attendue, modèle de référence, journal série, capture comparée) |
| AM | la logique est vérifiée automatiquement, un complément manuel couvre ce qui demande un jugement humain (aspect visuel, ergonomie, vrai clavier) |
| M | vérifiée seulement à la main : aucun test automatisé dans le dépôt |

La couverture automatisée est la part des fonctionnalités en mode A ou AM. La part manuelle seule est celle en mode M.
Les cas manuels sont décrits dans [`MANUEL.md`](MANUEL.md).

## Résultat

{chiffres}

Lecture : le taux par nombre de cas est très supérieur au taux fonctionnel parce que les cas automatisés sont fins
(une valeur limite, une partition) alors que les cas manuels sont des sessions complètes. Le taux fonctionnel est
la mesure à retenir.

Inventaire des cas automatisés : {inv["suites"]} fichiers de tests unitaires contenant {inv["cas_hote"]} cas nommés
(`h_run`), {inv["scenarios"]} scénarios QEMU et {inv["pytests"]} tests Python.

## Par sous-système

{par_sous_systeme(fonctions)}

## Traçabilité : exigence, technique, élément de couverture, cas

Pour les fonctionnalités dont la source (`matrice.toml`) porte `exigence`, `technique` et `element`, le tableau relie
l'exigence à la technique de 29119-4 qui la vérifie, à l'élément de couverture visé et aux noms de cas (`h_run` ou
fonction `test_`) que le générateur retrouve dans les fichiers cités. Un nom de cas introuvable fait échouer la
commande.

{tracabilite(fonctions)}

## Matrice complète

{tableau(fonctions)}
"""


def rendre_execution(m):
    if m["statut"] == "A exécuter":
        return ""
    lignes = [f"\nExécution du {m['date']} : {m['moyen']}.", "", f"Constat : {m['constat']}"]
    if m.get("observations"):
        lignes += ["", "Observations :", ""] + [f"- {o}" for o in m["observations"]]
    if m.get("incidents"):
        lignes += ["", "Incidents : " + ", ".join(m["incidents"]) + " (voir `../TESTS.md`)."]
    return "\n".join(lignes) + "\n"


def ecrire_manuel(fonctions, manuels):
    statuts = Counter(m["statut"] for m in manuels)
    duree = sum(m["duree"] for m in manuels)
    fiches = []
    pour = {}
    for f in fonctions:
        for r in f.get("manuels", []):
            pour.setdefault(r, []).append(f["id"])
    for m in manuels:
        etapes = "\n".join(f"{i}. {e}" for i, e in enumerate(m["etapes"], start=1))
        note = f"\nNote : {m['note']}\n" if m.get("note") else ""
        execution = rendre_execution(m)
        fiches.append(
            f"""### {m["id"]} : {m["titre"]}

- Fonctionnalités : {", ".join(pour[m["id"]])}
- Objectif : {m["objectif"]}
- Préconditions : {m["prerequis"]}
- Durée estimée : {m["duree"]} min
- Statut : {m["statut"]}

{etapes}

Résultat attendu : {m["attendu"]}
{note}{execution}"""
        )
    synthese = ", ".join(f"{statuts[s]} {s.lower()}" for s in STATUTS if statuts[s])
    joues = sum(statuts[s] for s in ("Réussi", "Échec", "Partiel"))
    return (
        f"""# Plan de tests manuels

Fichier généré par `tools/matrice_couverture.py` à partir de [`matrice.toml`](matrice.toml). Il couvre ce que
l'automatisation ne vérifie pas : jugement visuel, ergonomie, clavier et matériel réels, endurance, revue
linguistique, reproductibilité sur machine propre. Les liens avec les fonctionnalités sont dans
[`MATRICE.md`](MATRICE.md).

Exécution : {joues} cas sur {len(manuels)} ont été joués les 5 et 6 octobre 2026 sous QEMU/KVM, avec le clavier et la souris
injectés par le moniteur QEMU (`tools/manuel.py`) et des captures relues une par une. Aucun n'a été joué sur du matériel
réel ni par une personne devant l'écran. Restent à faire : la fluidité perçue (MT-19), le matériel réel (MT-18), l'endurance
de 8 h (MT-17 n'a duré que 10 minutes), la machine vierge (MT-24) et la comparaison avec de vraies captures d'époque
(MT-04). MT-14, MT-17, MT-22 et MT-23, en échec le 5 octobre, ont été rejoués le 6 octobre après correction (voir leur
fiche). Ces passages ont eu lieu sur l'ancien thème Luna : MT-25 (fidélité à la maquette verre, thèmes clair et sombre)
n'a pas encore été joué, et les fiches de rendu antérieures (MT-01, MT-04, MT-05, MT-14, MT-22) sont à rejouer sur le
thème verre. Un statut « Échec » renvoie à un incident
du registre de [`../TESTS.md`](../TESTS.md). À chaque nouvelle exécution, noter la date, le résultat et les anomalies
dans la fiche.

{len(manuels)} cas, durée estimée {duree // 60} h {duree % 60:02d} min. Statuts : {synthese}.

| ID | Cas | Durée (min) | Statut |
|----|-----|-------------|--------|
"""
        + "\n".join(f"| {m['id']} | {m['titre']} | {m['duree']} | {m['statut']} |" for m in manuels)
        + "\n\n## Fiches\n\n"
        + "\n".join(fiches)
    )


def main(argv):
    try:
        fonctions, manuels = charger()
    except Erreur as erreur:
        print(f"matrice : {erreur}", file=sys.stderr)
        return 1
    sorties = {
        MATRICE: ecrire_matrice(fonctions, manuels),
        MANUEL: ecrire_manuel(fonctions, manuels),
    }
    if "--orphelins" in argv:
        orphelins = tests_orphelins(fonctions)
        print("\n".join(orphelins))
        print(f"matrice : {len(orphelins)} fichiers de tests non cités", file=sys.stderr)
        return 0
    if "--check" in argv:
        perimes = [p.name for p, t in sorties.items() if not p.exists() or p.read_text() != t]
        if perimes:
            print(f"matrice : fichiers périmés {perimes}", file=sys.stderr)
            return 1
        print("matrice : à jour")
        return 0
    for chemin, texte in sorties.items():
        chemin.write_text(texte)
    resultat, _ = resume(fonctions, manuels)
    print(resultat)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
