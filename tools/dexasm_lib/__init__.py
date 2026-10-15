"""Assembleur DEX de VelumOS : texte lisible vers un fichier .dex (version 035)."""

from .ecriture import assembler
from .source import ErreurSource, lire_source


def assembler_texte(texte, fichier="<source>"):
    return assembler(lire_source(texte, fichier), fichier)


__all__ = ["ErreurSource", "assembler", "assembler_texte", "lire_source"]
