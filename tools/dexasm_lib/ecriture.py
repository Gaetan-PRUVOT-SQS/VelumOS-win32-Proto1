"""Tables triées et écriture du fichier DEX (version 035)."""

import hashlib
import struct
import zlib

from .code import Assembleur
from .source import ErreurSource

SANS_INDICE = 0xFFFFFFFF
TAILLE_ENTETE = 0x70
STATIQUE, PRIVE, CONSTRUCTEUR, NATIF, ABSTRAIT = 0x8, 0x2, 0x10000, 0x100, 0x400
CARTE = {
    "entete": 0x0000,
    "chaines": 0x0001,
    "types": 0x0002,
    "protos": 0x0003,
    "champs": 0x0004,
    "methodes": 0x0005,
    "classes": 0x0006,
    "carte": 0x1000,
    "listes": 0x1001,
    "donnees_classe": 0x2000,
    "code": 0x2001,
    "donnees_chaine": 0x2002,
    "tableaux": 0x2005,
}


def uleb(valeur):
    sortie = bytearray()
    while True:
        octet = valeur & 0x7F
        valeur >>= 7
        if valeur:
            sortie.append(octet | 0x80)
        else:
            sortie.append(octet)
            return bytes(sortie)


def sleb(valeur):
    sortie = bytearray()
    while True:
        octet = valeur & 0x7F
        valeur >>= 7
        fini = (valeur == 0 and not octet & 0x40) or (valeur == -1 and octet & 0x40)
        sortie.append(octet if fini else octet | 0x80)
        if fini:
            return bytes(sortie)


def unites_utf16(texte):
    brut = texte.encode("utf-16-le", "surrogatepass")
    return list(struct.unpack(f"<{len(brut) // 2}H", brut))


def mutf8(texte):
    sortie = bytearray()
    for unite in unites_utf16(texte):
        if 0 < unite < 0x80:
            sortie.append(unite)
        elif unite < 0x800:
            sortie += bytes((0xC0 | unite >> 6, 0x80 | unite & 0x3F))
        else:
            sortie += bytes((0xE0 | unite >> 12, 0x80 | unite >> 6 & 0x3F, 0x80 | unite & 0x3F))
    return bytes(sortie)


def abrege(parametres, retour):
    return "".join("L" if t[0] in "L[" else t for t in (retour, *parametres))


def octets_signes(valeur):
    for taille in range(1, 9):
        if -(1 << (8 * taille - 1)) <= valeur < 1 << (8 * taille - 1):
            return valeur.to_bytes(taille, "little", signed=True)
    raise ValueError(f"valeur {valeur} hors de 64 bits")


def octets_non_signes(valeur):
    return valeur.to_bytes(max(1, (valeur.bit_length() + 7) // 8), "little")


class Tables:
    def __init__(self):
        self.ensembles = {g: set() for g in ("chaine", "type", "proto", "champ", "methode")}
        self.indices = {}
        self.listes = {}

    def ajouter(self, genre, valeur):
        self.ensembles[genre].add(valeur)
        if genre == "type":
            self.ajouter("chaine", valeur)
        elif genre == "proto":
            parametres, retour = valeur
            self.ajouter("chaine", abrege(parametres, retour))
            for nom in (retour, *parametres):
                self.ajouter("type", nom)
        elif genre == "champ":
            self.ajouter("type", valeur[0])
            self.ajouter("chaine", valeur[1])
            self.ajouter("type", valeur[2])
        elif genre == "methode":
            self.ajouter("type", valeur[0])
            self.ajouter("chaine", valeur[1])
            self.ajouter("proto", (valeur[2], valeur[3]))

    def indice(self, genre, valeur):
        return self.indices[genre][valeur]

    def trier(self, genre, cle):
        self.listes[genre] = sorted(self.ensembles[genre], key=cle)
        self.indices[genre] = {v: i for i, v in enumerate(self.listes[genre])}

    def figer(self):
        chaine = lambda v: self.indice("chaine", v)  # noqa: E731
        type_ = lambda v: self.indice("type", v)  # noqa: E731
        self.trier("chaine", unites_utf16)
        self.trier("type", chaine)
        self.trier("proto", lambda p: (type_(p[1]), [type_(t) for t in p[0]]))
        proto = lambda m: self.indice("proto", (m[2], m[3]))  # noqa: E731
        self.trier("champ", lambda c: (type_(c[0]), chaine(c[1]), type_(c[2])))
        self.trier("methode", lambda m: (type_(m[0]), chaine(m[1]), proto(m)))
        for genre in ("type", "proto"):
            if len(self.listes[genre]) > 0x10000:
                raise ValueError(f"plus de 65536 entrées dans la table {genre}")


def ordonner_classes(classes):
    par_nom = {c.descripteur: c for c in classes}
    ordre, etat = [], {}

    def visiter(classe):
        if etat.get(classe.descripteur) == "fait":
            return
        if etat.get(classe.descripteur) == "en cours":
            raise ValueError(f"héritage circulaire autour de {classe.descripteur}")
        etat[classe.descripteur] = "en cours"
        for parent in (classe.super, *classe.interfaces):
            if parent in par_nom:
                visiter(par_nom[parent])
        etat[classe.descripteur] = "fait"
        ordre.append(classe)

    for classe in classes:
        visiter(classe)
    return ordre


class Ecrivain:
    def __init__(self, classes, fichier):
        self.fichier = fichier
        self.classes = classes
        self.tables = Tables()
        self.assembleurs = {}
        self.donnees = bytearray()
        self.base = 0
        self.sections = {}
        self.listes_types = {}

    def declarer(self):
        tables = self.tables
        for classe in self.classes:
            tables.ajouter("type", classe.descripteur)
            for nom in (classe.super, *classe.interfaces):
                if nom is not None:
                    tables.ajouter("type", nom)
            if classe.source is not None:
                tables.ajouter("chaine", classe.source)
            for champ in classe.champs:
                tables.ajouter("champ", (classe.descripteur, champ.nom, champ.type))
                if champ.valeur and isinstance(champ.valeur[0], str):
                    tables.ajouter("chaine", champ.valeur[0])
            for methode in classe.methodes:
                self.declarer_methode(classe, methode)

    def declarer_methode(self, classe, methode):
        cle = (classe.descripteur, methode.nom, tuple(methode.parametres), methode.retour)
        self.tables.ajouter("methode", cle)
        sans_code = methode.acces & (NATIF | ABSTRAIT)
        if sans_code and methode.corps:
            raise ErreurSource(self.fichier, methode.ligne, "méthode abstraite ou native avec code")
        if not sans_code:
            assembleur = Assembleur(self.fichier, methode, self.tables)
            assembleur.collecter()
            self.assembleurs[cle] = assembleur

    def aligner(self):
        self.donnees += b"\0" * (-len(self.donnees) % 4)

    def position(self):
        return self.base + len(self.donnees)

    def noter(self, section, nombre=1):
        debut, total = self.sections.get(section, (self.position(), 0))
        self.sections[section] = (debut, total + nombre)

    def liste_types(self, noms):
        if not noms:
            return 0
        cle = tuple(self.tables.indice("type", n) for n in noms)
        if cle not in self.listes_types:
            self.aligner()
            self.noter("listes")
            self.listes_types[cle] = self.position()
            self.donnees += struct.pack(f"<I{len(cle)}H", len(cle), *cle)
        return self.listes_types[cle]

    def ecrire_code(self, assembleur):
        code = assembleur.assembler()
        self.aligner()
        self.noter("code")
        debut = self.position()
        unites = code.unites
        self.donnees += struct.pack(
            "<4HII", code.registres, code.ins, code.outs, len(code.essais), 0, len(unites)
        )
        self.donnees += struct.pack(f"<{len(unites)}H", *unites)
        if code.essais:
            self.aligner()
            self.ecrire_essais(code.essais)
        return debut

    def ecrire_essais(self, essais):
        liste = bytearray(uleb(len(essais)))
        for debut, nombre, gestionnaires in essais:
            types = [g for g in gestionnaires if g[0] is not None]
            tout = [g for g in gestionnaires if g[0] is None]
            self.donnees += struct.pack("<IHH", debut, nombre, len(liste))
            liste += sleb(-len(types) if tout else len(types))
            for nom, adresse in types:
                liste += uleb(self.tables.indice("type", nom)) + uleb(adresse)
            if tout:
                liste += uleb(tout[0][1])
        self.donnees += liste

    def valeur_codee(self, champ):
        valeur = champ.valeur[0] if champ.valeur else None
        genre = champ.type
        if genre == "Z":
            return bytes((0x1F | bool(valeur) << 5,))
        if genre in ("F", "D"):
            brut = struct.pack("<f" if genre == "F" else "<d", valeur or 0.0)
            return bytes(((0x10 if genre == "F" else 0x11) | (len(brut) - 1) << 5,)) + brut
        if genre in ("B", "S", "I", "J"):
            brut = octets_signes(valeur or 0)
            limite = {"B": 1, "S": 2, "I": 4, "J": 8}[genre]
            if len(brut) > limite:
                raise ValueError(f"valeur {valeur} trop grande pour le type {genre}")
            return bytes(({"B": 0, "S": 2, "I": 4, "J": 6}[genre] | (len(brut) - 1) << 5,)) + brut
        if genre == "C":
            if not 0 <= (valeur or 0) <= 0xFFFF:
                raise ValueError(f"valeur {valeur} hors du type C")
            brut = octets_non_signes(valeur or 0)
            return bytes((0x03 | (len(brut) - 1) << 5,)) + brut
        if valeur is None:
            return b"\x1e"
        brut = octets_non_signes(self.tables.indice("chaine", valeur))
        return bytes((0x17 | (len(brut) - 1) << 5,)) + brut

    def membres(self, classe):
        tables = self.tables
        champs = sorted(
            classe.champs,
            key=lambda c: tables.indice("champ", (classe.descripteur, c.nom, c.type)),
        )

        def cle(m):
            return (classe.descripteur, m.nom, tuple(m.parametres), m.retour)

        methodes = sorted(classe.methodes, key=lambda m: tables.indice("methode", cle(m)))
        directes = [m for m in methodes if m.acces & (STATIQUE | PRIVE | CONSTRUCTEUR)]
        virtuelles = [m for m in methodes if not m.acces & (STATIQUE | PRIVE | CONSTRUCTEUR)]
        statiques = [c for c in champs if c.acces & STATIQUE]
        instance = [c for c in champs if not c.acces & STATIQUE]
        return statiques, instance, directes, virtuelles

    def donnees_classe(self, classe, groupes, codes):
        if not any(groupes):
            return 0
        self.noter("donnees_classe")
        debut = self.position()
        sortie = bytearray(b"".join(uleb(len(g)) for g in groupes))
        for groupe in groupes[:2]:
            precedent = 0
            for champ in groupe:
                indice = self.tables.indice("champ", (classe.descripteur, champ.nom, champ.type))
                sortie += uleb(indice - precedent) + uleb(champ.acces)
                precedent = indice
        for groupe in groupes[2:]:
            precedent = 0
            for methode in groupe:
                cle = (classe.descripteur, methode.nom, tuple(methode.parametres), methode.retour)
                indice = self.tables.indice("methode", cle)
                sortie += uleb(indice - precedent) + uleb(methode.acces) + uleb(codes.get(cle, 0))
                precedent = indice
        self.donnees += sortie
        return debut

    def valeurs_statiques(self, statiques):
        dernier = max((i for i, c in enumerate(statiques) if c.valeur), default=-1)
        if dernier < 0:
            return None
        sortie = bytearray(uleb(dernier + 1))
        for champ in statiques[: dernier + 1]:
            try:
                sortie += self.valeur_codee(champ)
            except ValueError as exc:
                raise ErreurSource(self.fichier, champ.ligne, str(exc)) from exc
        return bytes(sortie)

    def ecrire(self):
        self.declarer()
        tables = self.tables
        tables.figer()
        listes = tables.listes
        classes = ordonner_classes(self.classes)
        tailles = [
            4 * len(listes["chaine"]),
            4 * len(listes["type"]),
            12 * len(listes["proto"]),
            8 * len(listes["champ"]),
            8 * len(listes["methode"]),
            32 * len(classes),
        ]
        decalages = [TAILLE_ENTETE + sum(tailles[:i]) for i in range(6)]
        self.base = TAILLE_ENTETE + sum(tailles)
        protos = [self.liste_types(p[0]) for p in listes["proto"]]
        interfaces = [self.liste_types(c.interfaces) for c in classes]
        codes = {cle: self.ecrire_code(a) for cle, a in self.assembleurs.items()}
        groupes = [self.membres(c) for c in classes]
        donnees = [self.donnees_classe(c, g, codes) for c, g in zip(classes, groupes, strict=True)]
        chaines = []
        for texte in listes["chaine"]:
            self.noter("donnees_chaine")
            chaines.append(self.position())
            self.donnees += uleb(len(unites_utf16(texte))) + mutf8(texte) + b"\0"
        statiques = []
        for groupe in groupes:
            brut = self.valeurs_statiques(groupe[0])
            if brut is not None:
                self.noter("tableaux")
            statiques.append(self.position() if brut is not None else 0)
            self.donnees += brut or b""
        self.aligner()
        carte = self.position()
        tetes = bytearray()
        tetes += struct.pack(f"<{len(chaines)}I", *chaines)
        tetes += b"".join(struct.pack("<I", tables.indice("chaine", t)) for t in listes["type"])
        for (parametres, retour), liste in zip(listes["proto"], protos, strict=True):
            court = tables.indice("chaine", abrege(parametres, retour))
            tetes += struct.pack("<III", court, tables.indice("type", retour), liste)
        for classe, nom, genre in listes["champ"]:
            tetes += struct.pack(
                "<HHI",
                tables.indice("type", classe),
                tables.indice("type", genre),
                tables.indice("chaine", nom),
            )
        for classe, nom, parametres, retour in listes["methode"]:
            tetes += struct.pack(
                "<HHI",
                tables.indice("type", classe),
                tables.indice("proto", (parametres, retour)),
                tables.indice("chaine", nom),
            )
        for i, classe in enumerate(classes):
            tetes += struct.pack(
                "<8I",
                tables.indice("type", classe.descripteur),
                classe.acces,
                SANS_INDICE if classe.super is None else tables.indice("type", classe.super),
                interfaces[i],
                SANS_INDICE if classe.source is None else tables.indice("chaine", classe.source),
                0,
                donnees[i],
                statiques[i],
            )
        nombres = [len(listes[g]) for g in ("chaine", "type", "proto", "champ", "methode")]
        nombres.append(len(classes))
        return self.finir(tetes, carte, list(zip(nombres, decalages, strict=True)))

    def finir(self, tetes, carte, tables_fixes):
        entrees = [(CARTE["entete"], 1, 0)]
        noms = ("chaines", "types", "protos", "champs", "methodes", "classes")
        for nom, (nombre, decalage) in zip(noms, tables_fixes, strict=True):
            if nombre:
                entrees.append((CARTE[nom], nombre, decalage))
        for nom, (debut, nombre) in self.sections.items():
            entrees.append((CARTE[nom], nombre, debut))
        entrees.append((CARTE["carte"], 1, carte))
        entrees.sort(key=lambda e: e[2])
        self.donnees += struct.pack("<I", len(entrees))
        for genre, nombre, decalage in entrees:
            self.donnees += struct.pack("<HHII", genre, 0, nombre, decalage)
        total = self.base + len(self.donnees)
        entete = struct.pack("<8sI20sIII", b"dex\n035\0", 0, b"", total, TAILLE_ENTETE, 0x12345678)
        entete += struct.pack("<III", 0, 0, carte)
        for nombre, decalage in tables_fixes:
            entete += struct.pack("<II", nombre, decalage if nombre else 0)
        entete += struct.pack("<II", len(self.donnees), self.base)
        fichier = bytearray(entete + tetes + self.donnees)
        fichier[12:32] = hashlib.sha1(fichier[32:]).digest()
        fichier[8:12] = struct.pack("<I", zlib.adler32(bytes(fichier[12:])))
        return bytes(fichier)


def assembler(classes, fichier="<source>"):
    try:
        return Ecrivain(classes, fichier).ecrire()
    except ValueError as exc:
        raise ErreurSource(fichier, 0, str(exc)) from exc
