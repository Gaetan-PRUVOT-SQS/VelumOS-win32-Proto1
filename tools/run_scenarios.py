#!/usr/bin/env python3
import argparse
import concurrent.futures
import hashlib
import importlib.util
import json
import subprocess
import sys
import time
import traceback
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from vtest import (  # noqa: E402
    ICOUNT_DEFAUT,
    VM,
    ConfigurationIncompatible,
    Reglages,
    VMError,
)

RACINE = Path(__file__).resolve().parent.parent


def charger_scenarios(filtre, matrice=False):
    trouves = []
    for chemin in sorted((RACINE / "tests" / "qemu").glob("*.py")):
        if filtre and filtre not in chemin.stem:
            continue
        spec = importlib.util.spec_from_file_location(chemin.stem, chemin)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        if not hasattr(module, "run"):
            continue
        if hasattr(module, "CONFIGURATIONS") != matrice:
            continue
        trouves.append((chemin.stem, module))
    return trouves


def plus_recent(args):
    dates = [Path(args.kernel).stat().st_mtime]
    dates += [f.stat().st_mtime for f in Path(args.root).rglob("*") if f.is_file()]
    return max(dates)


def image_pour(args, cmdline, sortie):
    cle = hashlib.sha256(f"{cmdline}|{args.resolution}".encode()).hexdigest()[:10]
    image = sortie / f"disque-{cle}.img"
    if not image.exists() or image.stat().st_mtime < plus_recent(args):
        commande = [
            sys.executable,
            str(RACINE / "tools" / "mkimage.py"),
            "--noyau",
            args.kernel,
            "--sortie",
            str(image),
            "--initrd",
            args.root,
            "--resolution",
            args.resolution,
            "--cmdline",
            cmdline,
        ]
        subprocess.run(commande, check=True, capture_output=True)
    return image


def jouer(args, nom, module, firmware, sortie, image, reglages, configuration=None):
    debut = time.monotonic()
    memoire, smp = getattr(module, "MEMOIRE", None), getattr(module, "SMP", None)
    dossier, nom_vm = sortie / nom, f"{nom}-{firmware}"
    if configuration is not None:
        reglages = reglages.par_dessus(module.reglages(configuration))
        memoire, smp = None, None
        dossier, nom_vm = sortie / "matrice" / configuration.nom, "vm"
    machine = VM(
        image,
        dossier,
        nom=nom_vm,
        uefi=(firmware == "uefi"),
        memoire=memoire,
        smp=smp,
        extra=getattr(module, "QEMU_EXTRA", ()),
        reglages=reglages,
    )
    etat, retour = "OK", None
    try:
        with machine:
            if configuration is None:
                module.run(machine)
            else:
                retour = module.run(machine, configuration)
    except ConfigurationIncompatible as erreur:
        etat, retour = "IGNORE", str(erreur)
    except (VMError, AssertionError, OSError) as erreur:
        etat = "ECHEC"
        retour = {"message": f"{erreur}\n{traceback.format_exc(limit=3)}"}
        retour["observations"] = getattr(erreur, "observations", None)
    return {
        "nom": nom if configuration is None else configuration.nom,
        "firmware": firmware,
        "etat": etat,
        "retour": retour,
        "duree": time.monotonic() - debut,
        "avertissements": list(machine.avertissements),
        "ligne_qemu": " ".join(machine.commande_lancee),
        "dossier": str(dossier),
    }


def construire_parseur():
    parseur = argparse.ArgumentParser()
    parseur.add_argument("--kernel", required=True)
    parseur.add_argument("--root", required=True)
    parseur.add_argument("--out", required=True)
    parseur.add_argument("--jobs", type=int, default=4)
    parseur.add_argument("--filter", default="")
    parseur.add_argument("--resolution", default="1024x768")
    parseur.add_argument("--accel", choices=("tcg", "kvm"))
    parseur.add_argument("--cpu")
    parseur.add_argument("--smp", type=int)
    parseur.add_argument("--mem")
    parseur.add_argument("--machine")
    parseur.add_argument("--debug", action="store_true")
    parseur.add_argument("--icount", nargs="?", const=ICOUNT_DEFAUT)
    parseur.add_argument("--matrice", action="store_true")
    parseur.add_argument("--config", default="")
    parseur.add_argument("--rejouer-seul", action="store_true")
    return parseur


def reglages_depuis(args):
    explicites = Reglages(
        accel=args.accel,
        cpu=args.cpu,
        machine=args.machine,
        smp=args.smp,
        memoire=args.mem,
        debug=args.debug,
        icount=args.icount,
    )
    return Reglages.depuis_env().par_dessus(explicites)


def sortie_courte(chemin):
    absolu = Path(chemin).resolve()
    try:
        return absolu.relative_to(Path.cwd())
    except ValueError:
        return absolu


def travaux_classiques(args):
    travaux = []
    for nom, module in charger_scenarios(args.filter):
        for firmware in getattr(module, "FIRMWARES", ("bios", "uefi")):
            travaux.append((nom, module, firmware, None))
    return travaux


def travaux_matrice(args):
    travaux, ignores = [], []
    for nom, module in charger_scenarios(args.filter, matrice=True):
        for configuration in module.CONFIGURATIONS:
            if args.config and args.config not in configuration.nom:
                continue
            raison = module.ignorer_si(configuration)
            if raison:
                ignores.append((nom, configuration, raison))
                continue
            for firmware in getattr(module, "FIRMWARES", ("bios",)):
                travaux.append((nom, module, firmware, configuration))
    return travaux, ignores


def lancer(args, travaux, sortie, reglages, apres=None):
    images = {}
    for _, module, _, _ in travaux:
        cmdline = getattr(module, "CMDLINE", "quiet")
        if cmdline not in images:
            images[cmdline] = image_pour(args, cmdline, sortie)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futurs = [
            pool.submit(
                jouer,
                args,
                n,
                m,
                f,
                sortie,
                images[getattr(m, "CMDLINE", "quiet")],
                reglages,
                c,
            )
            for n, m, f, c in travaux
        ]
        resultats = []
        for futur in futurs:
            resultat = futur.result()
            if apres:
                apres(resultat)
            resultats.append(resultat)
        return resultats


def description_observations(observations):
    if not observations:
        return ""
    morceaux = []
    if "boot_ok_s" in observations:
        morceaux.append(f"boot ok {observations['boot_ok_s']:.3f}s")
    if "tsc_mesure" in observations:
        morceaux.append(
            f"TSC {observations['tsc_annonce']}/{observations['tsc_mesure']} Hz"
            f" {observations.get('horloge', '?')}"
        )
    return ", ".join(morceaux)


def afficher_resultat(resultat):
    etat, retour = resultat["etat"], resultat["retour"]
    detail = ""
    if etat == "IGNORE":
        detail = f"  ({retour})"
    elif etat == "OK" and isinstance(retour, dict):
        detail = f"  {description_observations(retour)}"
    nom = f"{resultat['nom']} [{resultat['firmware']}]"
    print(f"{etat:6} {nom} {resultat['duree']:5.1f}s{detail}", flush=True)
    for avertissement in resultat["avertissements"]:
        print(f"       avertissement : {avertissement}")
    if etat == "ECHEC":
        print(retour["message"], file=sys.stderr)
        print(f"       relancer : {resultat['ligne_qemu']}", file=sys.stderr)


def serialiser(resultats, ignores):
    lignes = []
    for r in resultats:
        retour = r["retour"]
        observations = retour.get("observations") if r["etat"] == "ECHEC" else retour
        lignes.append(
            {
                "configuration": r["nom"],
                "etat": r["etat"],
                "duree_s": round(r["duree"], 1),
                "message": retour["message"].splitlines()[0] if r["etat"] == "ECHEC" else None,
                "raison": retour if r["etat"] == "IGNORE" else None,
                "observations": observations if isinstance(observations, dict) else None,
                "ligne_qemu": r["ligne_qemu"],
                "rejoue": r.get("rejoue", False),
            }
        )
    for _nom, configuration, raison in ignores:
        lignes.append(
            {
                "configuration": configuration.nom,
                "etat": "IGNORE",
                "raison": raison,
                "ligne_qemu": configuration.ligne_qemu(),
            }
        )
    return lignes


def rejouer_seuls(args, resultats, travaux, sortie, reglages):
    index = {(c.nom if c else n, f): (n, m, f, c) for n, m, f, c in travaux}
    for position, resultat in enumerate(resultats):
        if resultat["etat"] != "ECHEC":
            continue
        travail = index[(resultat["nom"], resultat["firmware"])]
        print(f"rejeu seul : {resultat['nom']}", file=sys.stderr)
        second = lancer(args, [travail], sortie, reglages)[0]
        if second["etat"] == "OK":
            second["rejoue"] = True
            resultats[position] = second
            print(f"OK     {resultat['nom']} rejoué seul {second['duree']:5.1f}s")
        else:
            print(f"ECHEC  {resultat['nom']} échoue aussi rejoué seul")


def main(argv):
    args = construire_parseur().parse_args(argv)
    try:
        reglages = reglages_depuis(args)
    except VMError as erreur:
        print(f"réglages refusés : {erreur}", file=sys.stderr)
        return 2
    sortie = sortie_courte(args.out)
    sortie.mkdir(parents=True, exist_ok=True)
    if args.matrice:
        travaux, ignores = travaux_matrice(args)
    else:
        travaux, ignores = travaux_classiques(args), []
    for _nom, configuration, raison in ignores:
        print(f"IGNORE {configuration.nom}  ({raison})", flush=True)
    resultats = lancer(args, travaux, sortie, reglages, apres=afficher_resultat)
    if args.rejouer_seul:
        rejouer_seuls(args, resultats, travaux, sortie, reglages)
    echecs = sum(1 for r in resultats if r["etat"] == "ECHEC")
    ignores_runtime = sum(1 for r in resultats if r["etat"] == "IGNORE")
    ok = len(resultats) - echecs - ignores_runtime
    if args.matrice:
        (sortie / "matrice.json").write_text(
            json.dumps(serialiser(resultats, ignores), ensure_ascii=False, indent=1)
        )
        total_ignores = len(ignores) + ignores_runtime
        total = len(resultats) + len(ignores)
        print(f"matrice : {ok} OK, {echecs} KO, {total_ignores} ignorées sur {total}")
    else:
        print(f"scénarios : {ok}/{len(travaux)} réussis")
    return 1 if echecs else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
