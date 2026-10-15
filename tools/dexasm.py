#!/usr/bin/env python3
"""Assembleur DEX de VelumOS : texte lisible vers un fichier .dex (version 035).

dexasm.py SOURCE... --sortie classes.dex

Plusieurs sources donnent un seul fichier. Une erreur est signalée par
`fichier:ligne: message` sur la sortie d'erreur, avec un code de sortie 1.

Syntaxe (une directive ou une instruction par ligne, `#` ouvre un commentaire) :

  .classe [accès...] Lpaquet/Nom;        ouvre une classe (public, final, interface, abstract...)
  .super Lpaquet/Parent;                 défaut Ljava/lang/Object; ; `.super aucune` pour Object
  .realise Lpaquet/Interface;            une ligne par interface
  .source "Nom.dasm"                     nom de fichier source (facultatif)
  .champ [accès...] nom:Type [= valeur]  valeur initiale pour un champ static seulement
                                         (entier, 1.5, true, false, "chaîne", null)
  .methode [accès...] nom(Params)Retour  directe si static, private ou constructeur, sinon virtuelle
      .registres N                       nombre total de registres, arguments compris
      :etiquette
      instruction opérande, opérande     noms et formats de la spécification Dalvik
      .attrape Ltype; :début :fin :gestionnaire   bloc try [début, fin[ ; plusieurs lignes
      .attrape tout :début :fin :gestionnaire     pour un même bloc, `tout` en dernier
      .table-dense CLÉ :c0 :c1 ...       charge utile de packed-switch
      .table-creuse 1=:a 100=:b ...      charge utile de sparse-switch (triée par l'outil)
      .donnees-tableau LARGEUR v0 v1 ... charge utile de fill-array-data (largeur 1, 2, 4 ou 8)
  .fin

Opérandes : registres vN, ou pN pour les arguments (p0 = this hors static) ; littéraux
décimaux ou 0x, suffixe f ou d pour un flottant (const, const-wide, const/high16) ;
"chaîne" avec \\n \\t \\" \\\\ \\uXXXX ; type Lx/Y; ou [I ; champ Lx/Y;->nom:Type ;
méthode Lx/Y;->nom(Params)Retour ; invoke-* {v0, v1}, ref ; invoke-*/range {v0 .. v5}, ref ;
sauts et charges utiles par :etiquette. const/high16 et const-wide/high16 prennent la
valeur entière (0x7fff0000), dont seuls les 16 bits de poids fort sont non nuls.

L'outil calcule ins et outs, trie les tables, aligne les charges utiles (nop), écrit
map_list, Adler-32 et SHA-1. Non gérés : invoke-polymorphic, invoke-custom,
const-method-handle, const-method-type, annotations, informations de débogage.
"""

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from dexasm_lib import ErreurSource, assembler, lire_source  # noqa: E402


def assembler_fichiers(chemins):
    classes = []
    for chemin in chemins:
        try:
            texte = Path(chemin).read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError) as exc:
            raise ErreurSource(str(chemin), 0, f"lecture impossible : {exc}") from exc
        classes += lire_source(texte, str(chemin))
    noms = [c.descripteur for c in classes]
    if len(set(noms)) != len(noms):
        raise ErreurSource(str(chemins[-1]), 0, "classe définie dans deux sources")
    return assembler(classes, str(chemins[0]) if len(chemins) == 1 else "<sources>")


def main(arguments=None):
    analyseur = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    analyseur.add_argument("sources", nargs="+", metavar="SOURCE")
    analyseur.add_argument("--sortie", required=True, metavar="FICHIER.dex")
    options = analyseur.parse_args(arguments)
    try:
        brut = assembler_fichiers(options.sources)
        Path(options.sortie).write_bytes(brut)
    except ErreurSource as exc:
        print(f"dexasm: {exc}", file=sys.stderr)
        return 1
    except OSError as exc:
        print(f"dexasm: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
