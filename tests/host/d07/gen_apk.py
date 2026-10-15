#!/usr/bin/env python3
"""Fabrique les APK d'essai du lot d07 que tools/mkapk.py ne sait pas produire.

gen_apk.py DOSSIER           écrit les variantes à partir de DOSSIER/ok.apk et DOSSIER/cles
gen_apk.py --rang ZONE APK   affiche le rang d'un octet de la zone (entree, repertoire, fin, bloc)

Le bloc de signature v2 est réécrit ici sans reprendre le code de mkapk, pour que les deux
écrivains se contrôlent l'un l'autre. La seconde clé d'essai est créée par openssl dans DOSSIER.
"""

import io
import struct
import subprocess
import sys
import zipfile
from hashlib import sha256
from pathlib import Path

MAGIE = b"APK Sig Block 42"
ID_V2 = 0x7109871A
MORCEAU = 1 << 20
TROP = struct.pack("<I", 0xFFFFFFF0)


def openssl(arguments, entree=None):
    return subprocess.run(
        ["openssl", *arguments], input=entree, capture_output=True, check=True
    ).stdout


def prefixe(donnees):
    return struct.pack("<I", len(donnees)) + donnees


class Apk:
    def __init__(self, octets):
        self.octets = octets
        self.fin_pos = len(octets) - 22
        self.rep_pos = struct.unpack_from("<I", octets, self.fin_pos + 16)[0]
        taille = struct.unpack_from("<Q", octets, self.rep_pos - 24)[0]
        self.bloc_pos = self.rep_pos - taille - 8
        self.contenu = octets[: self.bloc_pos]
        self.repertoire = octets[self.rep_pos : self.fin_pos]
        self.fin = octets[self.fin_pos :]

    def condensat(self, fin):
        fin = fin[:16] + struct.pack("<I", len(self.contenu)) + fin[20:]
        morceaux = []
        for section in (self.contenu, self.repertoire, fin):
            for debut in range(0, len(section), MORCEAU):
                m = section[debut : debut + MORCEAU]
                morceaux.append(sha256(b"\xa5" + struct.pack("<I", len(m)) + m).digest())
        return sha256(b"\x5a" + struct.pack("<I", len(morceaux)) + b"".join(morceaux)).digest()

    def assembler(self, bloc, fin=None, bourrage=b""):
        fin = bytearray(fin or self.fin)
        struct.pack_into("<I", fin, 16, len(self.contenu) + len(bloc) + len(bourrage))
        return self.contenu + bloc + bourrage + self.repertoire + bytes(fin)


def signataire(cle, certificat, condensat, algo=0x0103):
    der = openssl(["x509", "-in", str(certificat / "cert.pem"), "-outform", "DER"])
    pem = openssl(["x509", "-in", str(cle / "cert.pem"), "-pubkey", "-noout"])
    publique = openssl(["pkey", "-pubin", "-outform", "DER"], pem)
    code = struct.pack("<I", algo)
    signees = prefixe(prefixe(code + prefixe(condensat))) + prefixe(prefixe(der)) + prefixe(b"")
    signature = openssl(["dgst", "-sha256", "-sign", str(cle / "cle.pem"), "-binary"], signees)
    return prefixe(signees) + prefixe(prefixe(code + prefixe(signature))) + prefixe(publique)


def bloc(signataires, autres=b""):
    valeur = prefixe(b"".join(prefixe(s) for s in signataires))
    paires = struct.pack("<QI", 4 + len(valeur), ID_V2) + valeur + autres
    taille = struct.pack("<Q", len(paires) + 24)
    return taille + paires + taille + MAGIE


def seconde_cle(dossier):
    cles = dossier / "cles2"
    cles.mkdir(exist_ok=True)
    if not (cles / "cle.pem").is_file():
        openssl(
            ["req", "-x509", "-newkey", "rsa:2048", "-nodes", "-sha256", "-days", "3650"]
            + ["-subj", "/CN=VelumOS essai 2", "-keyout", str(cles / "cle.pem")]
            + ["-out", str(cles / "cert.pem")]
        )
    return cles


def variantes_signature(apk, c1, c2):
    juste = apk.condensat(apk.fin)
    un = signataire(c1, c1, juste)
    magie = apk.fin[:20] + struct.pack("<H", 16) + MAGIE
    mot = apk.fin[:20] + struct.pack("<H", 7) + b"bonjour"
    inconnue = struct.pack("<QI", 12, 0x42726577) + bytes(8)
    sortie = {
        "refait.apk": apk.assembler(bloc([un])),
        "paire_inconnue.apk": apk.assembler(bloc([un], inconnue)),
        "algo_0104.apk": apk.assembler(bloc([signataire(c1, c1, bytes(64), 0x0104)])),
        "algo_inconnu.apk": apk.assembler(bloc([signataire(c1, c1, juste, 0x0999)])),
        "condensat_faux.apk": apk.assembler(bloc([signataire(c1, c1, bytes(32))])),
        "cle_autre.apk": apk.assembler(bloc([signataire(c2, c1, juste)])),
        "deux_signataires.apk": apk.assembler(bloc([un, un])),
        "zero_signataire.apk": apk.assembler(bloc([])),
        "decale.apk": apk.assembler(bloc([un]), bourrage=bytes(8)),
        "commentaire_magie.apk": apk.assembler(
            bloc([signataire(c1, c1, apk.condensat(magie))]), magie
        ),
        "commentaire.apk": apk.assembler(bloc([signataire(c1, c1, apk.condensat(mot))]), mot),
    }
    for numero, rang in enumerate((8, 20, 24, 28, 32)):
        rang += apk.bloc_pos
        sortie[f"long_{numero}.apk"] = apk.octets[:rang] + TROP + apk.octets[rang + 4 :]
    tailles = bytearray(apk.octets)
    tailles[apk.bloc_pos] ^= 8
    sortie["tailles.apk"] = bytes(tailles)
    return sortie


def archive(entrees):
    tampon = io.BytesIO()
    with zipfile.ZipFile(tampon, "w", zipfile.ZIP_DEFLATED) as z:
        for nom, donnees in entrees.items():
            z.writestr(zipfile.ZipInfo(nom), donnees, zipfile.ZIP_DEFLATED)
    return tampon.getvalue()


def variantes_archive(apk):
    with zipfile.ZipFile(io.BytesIO(apk.octets)) as z:
        base = {nom: z.read(nom) for nom in z.namelist()}
    sans_dex = {n: d for n, d in base.items() if n != "classes.dex"}
    sans_manifeste = {n: d for n, d in base.items() if n != "AndroidManifest.xml"}
    return {
        "zip_simple.apk": archive(base),
        "natif.apk": archive(base | {"lib/x86_64/libx.so": b"\x7fELF"}),
        "multidex.apk": archive(base | {"classes2.dex": b"dex\n"}),
        "nom.apk": archive(base | {"../sortie": b"x"}),
        "sans_dex.apk": archive(sans_dex),
        "sans_manifeste.apk": archive(sans_manifeste),
    }


def rang(zone, apk):
    rangs = {
        "entree": apk.bloc_pos // 2,
        "repertoire": apk.rep_pos + 12,
        "fin": apk.fin_pos + 12,
        "bloc": apk.bloc_pos + 100,
    }
    return rangs[zone]


def main(arguments):
    if len(arguments) == 3 and arguments[0] == "--rang":
        print(rang(arguments[1], Apk(Path(arguments[2]).read_bytes())))
        return 0
    if len(arguments) != 1:
        print(__doc__, file=sys.stderr)
        return 2
    dossier = Path(arguments[0])
    apk = Apk((dossier / "ok.apk").read_bytes())
    sortie = variantes_signature(apk, dossier / "cles", seconde_cle(dossier))
    sortie |= variantes_archive(apk)
    for nom, octets in sortie.items():
        (dossier / nom).write_bytes(octets)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
