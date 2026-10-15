#!/usr/bin/env python3
import struct
import sys

PT_LOAD = 1
PT_DYNAMIC = 2
PT_INTERP = 3
PT_TLS = 7
PT_GNU_STACK = 0x6474E551
PF_X = 1
PF_W = 2
ET_DYN = 3
EM_X86_64 = 62
DT_RELA = 7
DT_RELASZ = 8
DT_RELAENT = 9
R_X86_64_RELATIVE = 8
PAGE = 4096
MAX_LOADS = 16


def lire_entete(donnees):
    if donnees[:4] != b"\x7fELF" or donnees[4] != 2 or donnees[5] != 1:
        raise ValueError("pas un ELF64 petit-boutiste")
    champs = struct.unpack_from("<HHIQQQIHHHHHH", donnees, 16)
    cles = ("type", "machine", "version", "entree", "phoff", "shoff", "flags")
    cles += ("ehsize", "phentsize", "phnum", "shentsize", "shnum", "shstrndx")
    return dict(zip(cles, champs, strict=True))


def lire_segments(donnees, entete):
    segments = []
    for i in range(entete["phnum"]):
        debut = entete["phoff"] + i * entete["phentsize"]
        champs = struct.unpack_from("<IIQQQQQQ", donnees, debut)
        cles = ("type", "flags", "offset", "vaddr", "paddr", "filesz", "memsz", "align")
        segments.append(dict(zip(cles, champs, strict=True)))
    return segments


def segment_contenant(segments, adresse):
    for seg in segments:
        if seg["type"] == PT_LOAD and seg["vaddr"] <= adresse < seg["vaddr"] + seg["memsz"]:
            return seg
    return None


def lire_dynamique(donnees, seg):
    valeurs = {}
    for debut in range(seg["offset"], seg["offset"] + seg["filesz"], 16):
        tag, val = struct.unpack_from("<qQ", donnees, debut)
        if tag == 0:
            break
        valeurs[tag] = val
    return valeurs


def verifier_charges(segments):
    erreurs = []
    charges = [s for s in segments if s["type"] == PT_LOAD]
    if not charges or len(charges) > MAX_LOADS:
        erreurs.append(f"nombre de PT_LOAD invalide : {len(charges)}")
    fin_precedente = 0
    for seg in charges:
        if seg["flags"] & PF_W and seg["flags"] & PF_X:
            erreurs.append(f"segment W+X en {seg['vaddr']:#x}")
        if seg["vaddr"] % PAGE or seg["offset"] % PAGE or seg["align"] % PAGE:
            erreurs.append(f"segment non aligne sur 4096 : {seg['vaddr']:#x}")
        if seg["filesz"] > seg["memsz"]:
            erreurs.append(f"filesz > memsz en {seg['vaddr']:#x}")
        if seg["vaddr"] < fin_precedente:
            erreurs.append(f"chevauchement en {seg['vaddr']:#x}")
        fin_precedente = seg["vaddr"] + seg["memsz"]
    return erreurs


def verifier_relocations(donnees, segments, dyn):
    erreurs = []
    if DT_RELA not in dyn:
        return erreurs
    seg_rela = segment_contenant(segments, dyn[DT_RELA])
    if seg_rela is None or dyn.get(DT_RELAENT) != 24:
        return ["DT_RELA hors segment ou DT_RELAENT != 24"]
    debut = seg_rela["offset"] + dyn[DT_RELA] - seg_rela["vaddr"]
    for i in range(dyn.get(DT_RELASZ, 0) // 24):
        offset, info, addend = struct.unpack_from("<QQq", donnees, debut + i * 24)
        cible = segment_contenant(segments, offset)
        if info != R_X86_64_RELATIVE:
            erreurs.append(f"relocation {info:#x} autre que RELATIVE en {offset:#x}")
        if cible is None or not cible["flags"] & PF_W:
            erreurs.append(f"relocation hors segment inscriptible : {offset:#x}")
        if segment_contenant(segments, addend) is None:
            erreurs.append(f"addend hors image : {addend:#x}")
    return erreurs


def verifier(chemin):
    with open(chemin, "rb") as fichier:
        donnees = fichier.read()
    entete = lire_entete(donnees)
    segments = lire_segments(donnees, entete)
    erreurs = []
    if entete["type"] != ET_DYN or entete["machine"] != EM_X86_64:
        erreurs.append("ce n'est pas un ET_DYN x86-64")
    types = {s["type"] for s in segments}
    for interdit, nom in ((PT_INTERP, "PT_INTERP"), (PT_TLS, "PT_TLS")):
        if interdit in types:
            erreurs.append(f"{nom} present")
    for seg in segments:
        if seg["type"] == PT_GNU_STACK and seg["flags"] & PF_X:
            erreurs.append("pile executable")
    erreurs += verifier_charges(segments)
    entree = segment_contenant(segments, entete["entree"])
    if entree is None or not entree["flags"] & PF_X:
        erreurs.append("point d'entree hors segment executable")
    dynamiques = [s for s in segments if s["type"] == PT_DYNAMIC]
    if len(dynamiques) == 1:
        erreurs += verifier_relocations(donnees, segments, lire_dynamique(donnees, dynamiques[0]))
    return erreurs, len(segments)


def main(argv):
    total = 0
    for chemin in argv:
        erreurs, nombre = verifier(chemin)
        total += len(erreurs)
        for erreur in erreurs:
            print(f"{chemin} : {erreur}", file=sys.stderr)
        if not erreurs:
            print(f"{chemin} : ELF conforme ({nombre} en-tetes de programme)")
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
