#!/usr/bin/env python3
"""Fabrique un APK de test pour VelumOS et le signe au schéma v2 (RSA PKCS#1 v1.5 SHA-256).

mkapk.py DESCRIPTION.toml --dex classes.dex --sortie appli.apk --cles DOSSIER
         [--sans-signature] [--alterer OCTET]

La description TOML donne : paquet, version_code, version_nom, sdk_min, sdk_cible,
libelle (texte en clair ou "@string/nom"), activite (classe de l'activité principale,
filtre MAIN et LAUNCHER), permissions (liste), [chaines.defaut] et [chaines.fr].

L'outil écrit AndroidManifest.xml en XML binaire, resources.arsc (un paquet, type string,
configurations par défaut et fr), l'archive ZIP (manifeste et classes.dex compressés,
resources.arsc stocké et aligné sur 4 octets), puis le bloc de signature v2.
La clé et le certificat de test sont dérivés d'une graine publique (GRAINE_DEFAUT, ou la
variable VELUM_APK_GRAINE) et écrits dans --cles (cle.pem, cert.pem), jamais dans le dépôt hors
de build/. Même graine, même clé, même certificat, même APK : la construction est reproductible.
La clé est du matériel d'essai sans valeur : VelumOS ne s'en sert que pour comparer les
signataires entre eux, il n'embarque aucune ancre de confiance.
--sans-signature saute la signature ; --alterer inverse l'octet de ce rang après signature,
pour fabriquer un APK que la vérification refuse.
"""

import argparse
import base64
import hashlib
import math
import os
import re
import struct
import subprocess
import sys
import tomllib
import zlib
from dataclasses import dataclass, field
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent
ESPACE_ANDROID = "http://schemas.android.com/apk/res/android"
ATTRIBUTS = {
    "label": 0x01010001,
    "name": 0x01010003,
    "exported": 0x01010010,
    "minSdkVersion": 0x0101020C,
    "versionCode": 0x0101021B,
    "versionName": 0x0101021C,
    "targetSdkVersion": 0x01010270,
}
SANS = 0xFFFFFFFF
TYPE_REFERENCE, TYPE_CHAINE, TYPE_ENTIER, TYPE_BOOLEEN = 0x01, 0x03, 0x10, 0x12
ID_PAQUET, ID_TYPE_CHAINE = 0x7F, 0x01
RE_PAQUET = re.compile(r"^[A-Za-z][A-Za-z0-9_]*(\.[A-Za-z][A-Za-z0-9_]*)+$")
RE_CLASSE = re.compile(r"^[A-Za-z_][\w$]*(\.[A-Za-z_][\w$]*)*$")
RE_NOM_CHAINE = re.compile(r"^[a-z][a-z0-9_]*$")
ID_BLOC_V2 = 0x7109871A
ALGO_RSA_PKCS1_SHA256 = 0x0103
MAGIE_BLOC = b"APK Sig Block 42"
MORCEAU = 1 << 20
DATE_DOS = 0x0021
GRAINE_DEFAUT = "VelumOS : cle de test publique, sans valeur, derivee d'une graine fixe"
VARIABLE_GRAINE = "VELUM_APK_GRAINE"
BITS_PREMIER = 1024
EXPOSANT_PUBLIC = 65537
BASES_MILLER_RABIN = (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71)
PREFIXE_SHA256 = bytes.fromhex("3031300d060960864801650304020105000420")
OID_RSA = bytes.fromhex("06092a864886f70d010101")
OID_SHA256_RSA = bytes.fromhex("06092a864886f70d01010b")
OID_NOM_COMMUN = bytes.fromhex("0603550403")
OID_ORGANISATION = bytes.fromhex("060355040a")
OID_CONTRAINTES = bytes.fromhex("0603551d13")
DEBUT_VALIDITE = b"250101000000Z"
FIN_VALIDITE = b"491231235959Z"
SUJET_NOM_COMMUN = "VelumOS essai"
SUJET_ORGANISATION = "cle de test sans valeur"


class Erreur(Exception):
    pass


@dataclass
class Description:
    paquet: str
    version_code: int
    version_nom: str
    sdk_min: int
    sdk_cible: int
    libelle: str
    activite: str
    permissions: list
    chaines: dict
    chaines_fr: dict


@dataclass
class Element:
    nom: str
    attributs: list = field(default_factory=list)
    enfants: list = field(default_factory=list)


def exiger(table, cle, genre):
    if cle not in table:
        raise Erreur(f"clé « {cle} » absente de la description")
    valeur = table[cle]
    if not isinstance(valeur, genre) or isinstance(valeur, bool):
        raise Erreur(f"clé « {cle} » : type {genre.__name__} attendu")
    return valeur


def lire_chaines(table, nom):
    chaines = table.get(nom, {})
    if not isinstance(chaines, dict):
        raise Erreur(f"[chaines.{nom}] doit être une table")
    for cle, valeur in chaines.items():
        if not RE_NOM_CHAINE.match(cle) or not isinstance(valeur, str):
            raise Erreur(f"chaîne « {cle} » de [chaines.{nom}] invalide")
    return dict(chaines)


def lire_description(texte):
    try:
        table = tomllib.loads(texte)
    except tomllib.TOMLDecodeError as exc:
        raise Erreur(f"TOML invalide : {exc}") from exc
    chaines = table.get("chaines", {})
    if not isinstance(chaines, dict) or set(chaines) - {"defaut", "fr"}:
        raise Erreur("[chaines] n'accepte que les tables defaut et fr")
    description = Description(
        exiger(table, "paquet", str),
        exiger(table, "version_code", int),
        exiger(table, "version_nom", str),
        exiger(table, "sdk_min", int),
        exiger(table, "sdk_cible", int),
        exiger(table, "libelle", str),
        exiger(table, "activite", str),
        table.get("permissions", []),
        lire_chaines(chaines, "defaut"),
        lire_chaines(chaines, "fr"),
    )
    valider(description)
    return description


def valider(d):
    if not RE_PAQUET.match(d.paquet) or len(d.paquet) > 127:
        raise Erreur(f"nom de paquet invalide « {d.paquet} »")
    if not RE_CLASSE.match(d.activite):
        raise Erreur(f"classe d'activité invalide « {d.activite} »")
    for nom, valeur in (("version_code", d.version_code), ("sdk_min", d.sdk_min)):
        if not 1 <= valeur <= 0x7FFFFFFF:
            raise Erreur(f"{nom} hors de 1..2147483647")
    if not d.sdk_min <= d.sdk_cible <= 0x7FFFFFFF:
        raise Erreur("sdk_cible doit être au moins sdk_min")
    if not isinstance(d.permissions, list) or not all(isinstance(p, str) for p in d.permissions):
        raise Erreur("permissions doit être une liste de chaînes")
    if set(d.chaines_fr) - set(d.chaines):
        raise Erreur("chaîne fr sans valeur par défaut")
    if d.libelle.startswith("@string/") and d.libelle[8:] not in d.chaines:
        raise Erreur(f"libellé : chaîne « {d.libelle[8:]} » absente de [chaines.defaut]")


def noms_chaines(d):
    return sorted(d.chaines)


def identifiant_chaine(d, nom):
    return ID_PAQUET << 24 | ID_TYPE_CHAINE << 16 | noms_chaines(d).index(nom)


def longueur8(nombre):
    if nombre > 0x7FFF:
        raise Erreur("chaîne de ressource trop longue")
    return bytes((nombre >> 8 | 0x80, nombre & 0xFF)) if nombre > 0x7F else bytes((nombre,))


def reserve_chaines(chaines, utf8):
    corps, decalages = bytearray(), []
    for texte in chaines:
        decalages.append(len(corps))
        utf16 = texte.encode("utf-16-le")
        if utf8:
            brut = texte.encode("utf-8")
            corps += longueur8(len(utf16) // 2) + longueur8(len(brut)) + brut + b"\0"
        else:
            if len(utf16) // 2 > 0x7FFF:
                raise Erreur("chaîne de manifeste trop longue")
            corps += struct.pack("<H", len(utf16) // 2) + utf16 + b"\0\0"
    corps += b"\0" * (-len(corps) % 4)
    debut = 28 + 4 * len(chaines)
    entete = struct.pack(
        "<HHI5I", 0x0001, 28, debut + len(corps), len(chaines), 0, 0x100 if utf8 else 0, debut, 0
    )
    return entete + struct.pack(f"<{len(decalages)}I", *decalages) + bytes(corps)


class Manifeste:
    def __init__(self, racine):
        self.racine = racine
        self.attributs = sorted(self.noms_systeme(racine), key=ATTRIBUTS.get)
        self.chaines = list(self.attributs)
        self.corps = bytearray()

    def noms_systeme(self, element):
        noms = {a[0] for a in element.attributs if a[1]}
        for enfant in element.enfants:
            noms |= self.noms_systeme(enfant)
        return noms

    def indice(self, texte):
        debut = 0 if texte in self.attributs else len(self.attributs)
        if texte not in self.chaines[debut:]:
            self.chaines.append(texte)
        return self.chaines.index(texte, debut)

    def noeud(self, genre, charge):
        self.corps += struct.pack("<HHIII", genre, 16, 16 + len(charge), 1, SANS) + charge

    def valeur(self, genre, valeur):
        if genre == TYPE_CHAINE:
            indice = self.indice(valeur)
            return indice, indice
        if genre == TYPE_BOOLEEN:
            return SANS, SANS if valeur else 0
        return SANS, valeur

    def ecrire_element(self, element):
        espace = self.indice(ESPACE_ANDROID)
        ordonnes = sorted(element.attributs, key=lambda a: ATTRIBUTS[a[0]] if a[1] else 1 << 32)
        charge = struct.pack(
            "<II6H", SANS, self.indice(element.nom), 20, 20, len(ordonnes), 0, 0, 0
        )
        for nom, systeme, genre, valeur in ordonnes:
            brut, donnee = self.valeur(genre, valeur)
            charge += struct.pack("<III", espace if systeme else SANS, self.indice(nom), brut)
            charge += struct.pack("<HBBI", 8, 0, genre, donnee)
        self.noeud(0x0102, charge)
        for enfant in element.enfants:
            self.ecrire_element(enfant)
        self.noeud(0x0103, struct.pack("<II", SANS, self.indice(element.nom)))

    def octets(self):
        espace = struct.pack("<II", self.indice("android"), self.indice(ESPACE_ANDROID))
        self.noeud(0x0100, espace)
        self.ecrire_element(self.racine)
        self.noeud(0x0101, espace)
        identifiants = [ATTRIBUTS[n] for n in self.attributs]
        carte = struct.pack("<HHI", 0x0180, 8, 8 + 4 * len(identifiants))
        carte += struct.pack(f"<{len(identifiants)}I", *identifiants)
        contenu = reserve_chaines(self.chaines, utf8=False) + carte + bytes(self.corps)
        return struct.pack("<HHI", 0x0003, 8, 8 + len(contenu)) + contenu


def arbre_manifeste(d):
    def systeme(nom, genre, valeur):
        return (nom, True, genre, valeur)

    if d.libelle.startswith("@string/"):
        libelle = systeme("label", TYPE_REFERENCE, identifiant_chaine(d, d.libelle[8:]))
    else:
        libelle = systeme("label", TYPE_CHAINE, d.libelle)
    filtre = Element(
        "intent-filter",
        [],
        [
            Element("action", [systeme("name", TYPE_CHAINE, "android.intent.action.MAIN")]),
            Element("category", [systeme("name", TYPE_CHAINE, "android.intent.category.LAUNCHER")]),
        ],
    )
    activite = Element(
        "activity",
        [systeme("name", TYPE_CHAINE, d.activite), systeme("exported", TYPE_BOOLEEN, True)],
        [filtre],
    )
    enfants = [
        Element(
            "uses-sdk",
            [
                systeme("minSdkVersion", TYPE_ENTIER, d.sdk_min),
                systeme("targetSdkVersion", TYPE_ENTIER, d.sdk_cible),
            ],
        )
    ]
    enfants += [
        Element("uses-permission", [systeme("name", TYPE_CHAINE, p)]) for p in d.permissions
    ]
    enfants.append(Element("application", [libelle], [activite]))
    racine = [
        systeme("versionCode", TYPE_ENTIER, d.version_code),
        systeme("versionName", TYPE_CHAINE, d.version_nom),
        ("package", False, TYPE_CHAINE, d.paquet),
    ]
    return Element("manifest", racine, enfants)


def manifeste_binaire(d):
    return Manifeste(arbre_manifeste(d)).octets()


def bloc_type(noms, valeurs, langue, indices):
    config = struct.pack("<IHH2s2s", 64, 0, 0, langue, b"\0\0").ljust(64, b"\0")
    decalages, entrees = [], bytearray()
    for cle, nom in enumerate(noms):
        if nom not in valeurs:
            decalages.append(SANS)
            continue
        decalages.append(len(entrees))
        entrees += struct.pack("<HHIHBBI", 8, 0, cle, 8, 0, TYPE_CHAINE, indices[valeurs[nom]])
    taille_entete = 20 + len(config)
    debut = taille_entete + 4 * len(noms)
    entete = struct.pack(
        "<HHIBBHII",
        0x0201,
        taille_entete,
        debut + len(entrees),
        ID_TYPE_CHAINE,
        0,
        0,
        len(noms),
        debut,
    )
    return entete + config + struct.pack(f"<{len(noms)}I", *decalages) + bytes(entrees)


def ressources_binaires(d):
    noms = noms_chaines(d)
    textes = sorted(set(d.chaines.values()) | set(d.chaines_fr.values()))
    indices = {t: i for i, t in enumerate(textes)}
    types = reserve_chaines(["string"], utf8=True)
    cles = reserve_chaines(noms, utf8=True)
    drapeaux = [0x0004 if nom in d.chaines_fr else 0 for nom in noms]
    spec = struct.pack("<HHIBBHI", 0x0202, 16, 16 + 4 * len(noms), ID_TYPE_CHAINE, 0, 0, len(noms))
    spec += struct.pack(f"<{len(noms)}I", *drapeaux)
    corps = types + cles + spec + bloc_type(noms, d.chaines, b"\0\0", indices)
    if d.chaines_fr:
        corps += bloc_type(noms, d.chaines_fr, b"fr", indices)
    nom_paquet = d.paquet.encode("utf-16-le").ljust(256, b"\0")
    paquet = struct.pack("<HHII", 0x0200, 288, 288 + len(corps), ID_PAQUET) + nom_paquet
    paquet += struct.pack("<5I", 288, 1, 288 + len(types), len(noms), 0) + corps
    contenu = reserve_chaines(textes, utf8=True) + paquet
    return struct.pack("<HHII", 0x0002, 12, 12 + len(contenu), 1) + contenu


def deflate_brut(donnees):
    moteur = zlib.compressobj(9, zlib.DEFLATED, -15)
    return moteur.compress(donnees) + moteur.flush()


def archive_zip(entrees):
    sortie, repertoire = bytearray(), bytearray()
    for nom, donnees, compresser in entrees:
        nom_brut = nom.encode("utf-8")
        charge = deflate_brut(donnees) if compresser else donnees
        methode = 8 if compresser else 0
        bourrage = 0 if compresser else -(len(sortie) + 30 + len(nom_brut)) % 4
        commun = struct.pack(
            "<HHHHHIII",
            20,
            0,
            methode,
            0,
            DATE_DOS,
            zlib.crc32(donnees),
            len(charge),
            len(donnees),
        )
        repertoire += struct.pack("<IH", 0x02014B50, 20) + commun
        repertoire += struct.pack("<HHHHHII", len(nom_brut), 0, 0, 0, 0, 0, len(sortie)) + nom_brut
        sortie += struct.pack("<I", 0x04034B50) + commun
        sortie += struct.pack("<HH", len(nom_brut), bourrage) + nom_brut + b"\0" * bourrage + charge
    fin = struct.pack(
        "<IHHHHIIH", 0x06054B50, 0, 0, len(entrees), len(entrees), len(repertoire), len(sortie), 0
    )
    return bytes(sortie + repertoire + fin)


def prefixe(donnees):
    return struct.pack("<I", len(donnees)) + donnees


def condensat_apk(contenu, repertoire, fin):
    condensats = []
    for section in (contenu, repertoire, fin):
        for debut in range(0, len(section), MORCEAU):
            morceau = section[debut : debut + MORCEAU]
            tete = b"\xa5" + struct.pack("<I", len(morceau))
            condensats.append(hashlib.sha256(tete + morceau).digest())
    tete = b"\x5a" + struct.pack("<I", len(condensats))
    return hashlib.sha256(tete + b"".join(condensats)).digest()


def openssl(arguments, entree=None):
    try:
        resultat = subprocess.run(["openssl", *arguments], input=entree, capture_output=True)
    except OSError as exc:
        raise Erreur(f"openssl introuvable : {exc}") from exc
    if resultat.returncode != 0:
        raise Erreur(f"openssl {arguments[0]} : {resultat.stderr.decode(errors='replace').strip()}")
    return resultat.stdout


def dossier_cles(chemin):
    dossier = Path(chemin).resolve()
    if dossier.is_relative_to(RACINE) and not dossier.is_relative_to(RACINE / "build"):
        raise Erreur("le dossier des clés doit être hors du dépôt ou sous build/")
    dossier.mkdir(parents=True, exist_ok=True)
    return dossier


def octets_derives(graine, etiquette, taille):
    sortie = b""
    compteur = 0
    while len(sortie) < taille:
        sortie += hashlib.sha256(f"{graine}\0{etiquette}\0{compteur}".encode()).digest()
        compteur += 1
    return sortie[:taille]


def petits_premiers(limite=2000):
    return [n for n in range(3, limite, 2) if all(n % d for d in range(3, math.isqrt(n) + 1, 2))]


def est_premier(candidat, petits):
    for premier in petits:
        if candidat % premier == 0:
            return candidat == premier
    impair, puissance = candidat - 1, 0
    while impair % 2 == 0:
        impair //= 2
        puissance += 1
    for base in BASES_MILLER_RABIN:
        x = pow(base, impair, candidat)
        if x in (1, candidat - 1):
            continue
        for _ in range(puissance - 1):
            x = x * x % candidat
            if x == candidat - 1:
                break
        else:
            return False
    return True


def premier_depuis_graine(graine, nom, petits):
    brut = octets_derives(graine, nom, BITS_PREMIER // 8)
    candidat = int.from_bytes(brut, "big") | (3 << (BITS_PREMIER - 2)) | 1
    while not (est_premier(candidat, petits) and (candidat - 1) % EXPOSANT_PUBLIC != 0):
        candidat += 2
    return candidat


def der(etiquette, contenu):
    taille = len(contenu)
    if taille < 0x80:
        longueur = bytes([taille])
    else:
        octets = taille.to_bytes((taille.bit_length() + 7) // 8, "big")
        longueur = bytes([0x80 | len(octets)]) + octets
    return bytes([etiquette]) + longueur + contenu


def der_entier(valeur):
    octets = valeur.to_bytes(valeur.bit_length() // 8 + 1, "big")
    return der(0x02, octets)


def der_sequence(*elements):
    return der(0x30, b"".join(elements))


def der_nom(valeur):
    return der(0x0C, valeur.encode())


def pem(etiquette, contenu):
    corps = base64.b64encode(contenu).decode()
    lignes = [corps[i : i + 64] for i in range(0, len(corps), 64)]
    return "\n".join([f"-----BEGIN {etiquette}-----", *lignes, f"-----END {etiquette}-----", ""])


def cle_privee_der(n, d, p, q):
    entiers = (0, n, EXPOSANT_PUBLIC, d, p, q, d % (p - 1), d % (q - 1), pow(q, -1, p))
    rsa = der_sequence(*(der_entier(v) for v in entiers))
    algo = der_sequence(OID_RSA, der(0x05, b""))
    return der_sequence(der_entier(0), algo, der(0x04, rsa))


def certificat_der(graine, n, d):
    algo = der_sequence(OID_SHA256_RSA, der(0x05, b""))
    nom_commun = der(0x31, der_sequence(OID_NOM_COMMUN, der_nom(SUJET_NOM_COMMUN)))
    organisation = der(0x31, der_sequence(OID_ORGANISATION, der_nom(SUJET_ORGANISATION)))
    sujet = der_sequence(nom_commun, organisation)
    validite = der_sequence(der(0x17, DEBUT_VALIDITE), der(0x17, FIN_VALIDITE))
    publique = der_sequence(der_entier(n), der_entier(EXPOSANT_PUBLIC))
    spki = der_sequence(der_sequence(OID_RSA, der(0x05, b"")), der(0x03, b"\0" + publique))
    contraintes = der_sequence(
        OID_CONTRAINTES, der(0x01, b"\xff"), der(0x04, der(0x30, der(0x01, b"\xff")))
    )
    numero = int.from_bytes(octets_derives(graine, "serie", 16), "big") >> 1 or 1
    corps = der_sequence(
        der(0xA0, der_entier(2)),
        der_entier(numero),
        algo,
        sujet,
        validite,
        sujet,
        spki,
        der(0xA3, der_sequence(contraintes)),
    )
    condensat = PREFIXE_SHA256 + hashlib.sha256(corps).digest()
    taille = (n.bit_length() + 7) // 8
    bourre = b"\x00\x01" + b"\xff" * (taille - len(condensat) - 3) + b"\x00" + condensat
    signature = pow(int.from_bytes(bourre, "big"), d, n).to_bytes(taille, "big")
    return der_sequence(corps, algo, der(0x03, b"\0" + signature))


def materiel_depuis_graine(graine):
    petits = petits_premiers()
    p = premier_depuis_graine(graine, "p", petits)
    q = premier_depuis_graine(graine, "q", petits)
    if p == q:
        raise Erreur("graine dégénérée : deux premiers identiques")
    p, q = max(p, q), min(p, q)
    d = pow(EXPOSANT_PUBLIC, -1, math.lcm(p - 1, q - 1))
    n = p * q
    cle = pem("PRIVATE KEY", cle_privee_der(n, d, p, q))
    certificat = pem("CERTIFICATE", certificat_der(graine, n, d))
    return cle, certificat


def ecrire_atomiquement(chemin, texte, droits):
    provisoire = chemin.with_name(f"{chemin.name}.{os.getpid()}.nouveau")
    provisoire.write_text(texte, encoding="ascii")
    provisoire.chmod(droits)
    os.replace(provisoire, chemin)


def graine_courante():
    return os.environ.get(VARIABLE_GRAINE) or GRAINE_DEFAUT


def preparer_cles(chemin, graine=None):
    graine = graine_courante() if graine is None else graine
    dossier = dossier_cles(chemin)
    cle, certificat, marque = dossier / "cle.pem", dossier / "cert.pem", dossier / "graine.sha256"
    empreinte = hashlib.sha256(graine.encode()).hexdigest()
    deja_prets = cle.is_file() and certificat.is_file() and marque.is_file()
    if deja_prets and marque.read_text(encoding="ascii").strip() == empreinte:
        return cle, certificat
    texte_cle, texte_certificat = materiel_depuis_graine(graine)
    ecrire_atomiquement(cle, texte_cle, 0o600)
    ecrire_atomiquement(certificat, texte_certificat, 0o644)
    ecrire_atomiquement(marque, empreinte + "\n", 0o644)
    return cle, certificat


def bloc_signature(condensat, cle, certificat):
    der = openssl(["x509", "-in", str(certificat), "-outform", "DER"])
    pem_publique = openssl(["x509", "-in", str(certificat), "-pubkey", "-noout"])
    publique = openssl(["pkey", "-pubin", "-outform", "DER"], pem_publique)
    algo = struct.pack("<I", ALGO_RSA_PKCS1_SHA256)
    signees = prefixe(prefixe(algo + prefixe(condensat))) + prefixe(prefixe(der)) + prefixe(b"")
    signature = openssl(["dgst", "-sha256", "-sign", str(cle), "-binary"], signees)
    signataire = prefixe(signees) + prefixe(prefixe(algo + prefixe(signature))) + prefixe(publique)
    valeur = prefixe(prefixe(signataire))
    paire = struct.pack("<QI", 4 + len(valeur), ID_BLOC_V2) + valeur
    taille = struct.pack("<Q", len(paire) + 24)
    return taille + paire + taille + MAGIE_BLOC


def signer(archive, cle, certificat):
    debut_fin = len(archive) - 22
    debut_repertoire = struct.unpack_from("<I", archive, debut_fin + 16)[0]
    contenu, repertoire = archive[:debut_repertoire], archive[debut_repertoire:debut_fin]
    fin = bytearray(archive[debut_fin:])
    bloc = bloc_signature(condensat_apk(contenu, repertoire, bytes(fin)), cle, certificat)
    struct.pack_into("<I", fin, 16, debut_repertoire + len(bloc))
    return contenu + bloc + repertoire + bytes(fin)


def alterer(apk, rang):
    if not 0 <= rang < len(apk):
        raise Erreur(f"--alterer {rang} hors du fichier ({len(apk)} octets)")
    modifie = bytearray(apk)
    modifie[rang] ^= 0xFF
    return bytes(modifie)


def fabriquer(description, dex, cles=None, rang_altere=None):
    entrees = [
        ("AndroidManifest.xml", manifeste_binaire(description), True),
        ("classes.dex", dex, True),
        ("resources.arsc", ressources_binaires(description), False),
    ]
    apk = archive_zip(entrees)
    if cles is not None:
        apk = signer(apk, *preparer_cles(cles))
    if rang_altere is not None:
        apk = alterer(apk, rang_altere)
    return apk


def main(arguments=None):
    analyseur = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    analyseur.add_argument("description", metavar="DESCRIPTION.toml")
    analyseur.add_argument("--dex", required=True, metavar="classes.dex")
    analyseur.add_argument("--sortie", required=True, metavar="FICHIER.apk")
    analyseur.add_argument("--cles", metavar="DOSSIER")
    analyseur.add_argument("--sans-signature", action="store_true")
    analyseur.add_argument("--alterer", type=int, metavar="OCTET")
    options = analyseur.parse_args(arguments)
    try:
        if not options.sans_signature and options.cles is None:
            raise Erreur("--cles DOSSIER est requis pour signer (ou --sans-signature)")
        description = lire_description(Path(options.description).read_text(encoding="utf-8"))
        dex = Path(options.dex).read_bytes()
        if dex[:4] != b"dex\n":
            raise Erreur(f"{options.dex} n'est pas un fichier DEX")
        cles = None if options.sans_signature else options.cles
        Path(options.sortie).write_bytes(fabriquer(description, dex, cles, options.alterer))
    except (Erreur, OSError, UnicodeDecodeError) as exc:
        print(f"mkapk: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
