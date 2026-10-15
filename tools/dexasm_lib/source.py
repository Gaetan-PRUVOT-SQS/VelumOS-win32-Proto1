"""Lecture du texte d'assemblage : classes, champs, méthodes, instructions."""

import re
from dataclasses import dataclass, field

ACCES = {
    "public": 0x1,
    "private": 0x2,
    "protected": 0x4,
    "static": 0x8,
    "final": 0x10,
    "synchronized": 0x20,
    "volatile": 0x40,
    "bridge": 0x40,
    "transient": 0x80,
    "varargs": 0x80,
    "native": 0x100,
    "interface": 0x200,
    "abstract": 0x400,
    "strict": 0x800,
    "synthetic": 0x1000,
    "enum": 0x4000,
}
CONSTRUCTEUR = 0x10000
RE_TYPE = re.compile(r"\[*(?:[ZBSCIJFDV]|L[^;\s]+;)")
RE_PROTO = re.compile(r"^([^\s(]+)\(([^)]*)\)(\S+)$")
RE_ETIQUETTE = re.compile(r"^:[A-Za-z_][\w.$]*$")
ECHAPPEMENTS = {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"', "0": "\0"}


class ErreurSource(Exception):
    def __init__(self, fichier, ligne, message):
        super().__init__(f"{fichier}:{ligne}: {message}")
        self.fichier = fichier
        self.ligne = ligne
        self.message = message


@dataclass
class Instruction:
    ligne: int
    nom: str
    operandes: list
    adresse: int = 0


@dataclass
class Charge:
    ligne: int
    genre: str
    arguments: list
    adresse: int = 0
    origine: int = -1


@dataclass
class Etiquette:
    ligne: int
    nom: str


@dataclass
class Essai:
    ligne: int
    type: str | None
    debut: str
    fin: str
    gestionnaire: str


@dataclass
class Methode:
    ligne: int
    acces: int
    nom: str
    parametres: list
    retour: str
    registres: int = -1
    corps: list = field(default_factory=list)
    essais: list = field(default_factory=list)


@dataclass
class Champ:
    ligne: int
    acces: int
    nom: str
    type: str
    valeur: object = None


@dataclass
class Classe:
    ligne: int
    acces: int
    descripteur: str
    super: str | None = "Ljava/lang/Object;"
    interfaces: list = field(default_factory=list)
    source: str | None = None
    champs: list = field(default_factory=list)
    methodes: list = field(default_factory=list)


def est_type(texte):
    return RE_TYPE.fullmatch(texte) is not None


def decouper_types(texte):
    types, pos = [], 0
    while pos < len(texte):
        trouve = RE_TYPE.match(texte, pos)
        if trouve is None:
            return None
        types.append(trouve.group())
        pos = trouve.end()
    return types


def lire_chaine(texte):
    if len(texte) < 2 or texte[0] != '"' or texte[-1] != '"':
        raise ValueError("chaîne entre guillemets attendue")
    sortie, pos, corps = [], 0, texte[1:-1]
    while pos < len(corps):
        car = corps[pos]
        if car == '"':
            raise ValueError("guillemet non échappé dans la chaîne")
        if car != "\\":
            sortie.append(car)
            pos += 1
            continue
        if pos + 1 >= len(corps):
            raise ValueError("échappement incomplet")
        code = corps[pos + 1]
        if code == "u":
            sortie.append(chr(int(corps[pos + 2 : pos + 6], 16)))
            pos += 6
        elif code in ECHAPPEMENTS:
            sortie.append(ECHAPPEMENTS[code])
            pos += 2
        else:
            raise ValueError(f"échappement inconnu \\{code}")
    return "".join(sortie)


def retirer_commentaire(ligne):
    dans_chaine, pos = False, 0
    while pos < len(ligne):
        car = ligne[pos]
        if dans_chaine and car == "\\":
            pos += 1
        elif car == '"':
            dans_chaine = not dans_chaine
        elif car == "#" and not dans_chaine:
            return ligne[:pos]
        pos += 1
    return ligne


def decouper_operandes(texte):
    morceaux, courant, dans_chaine, niveau, pos = [], [], False, 0, 0
    while pos < len(texte):
        car = texte[pos]
        if dans_chaine and car == "\\":
            courant.append(texte[pos : pos + 2])
            pos += 2
            continue
        if car == '"':
            dans_chaine = not dans_chaine
        elif not dans_chaine and car == "{":
            niveau += 1
        elif not dans_chaine and car == "}":
            niveau -= 1
        if car == "," and not dans_chaine and niveau == 0:
            morceaux.append("".join(courant).strip())
            courant = []
        else:
            courant.append(car)
        pos += 1
    if dans_chaine or niveau != 0:
        raise ValueError("guillemet ou accolade non fermé")
    reste = "".join(courant).strip()
    if reste or morceaux:
        morceaux.append(reste)
    if any(not m for m in morceaux):
        raise ValueError("opérande vide")
    return morceaux


def lire_acces(mots):
    acces = 0
    for mot in mots:
        if mot not in ACCES:
            raise ValueError(f"mot d'accès inconnu « {mot} »")
        acces |= ACCES[mot]
    return acces


def lire_valeur(type_champ, texte):
    if texte == "null":
        return None
    if texte.startswith('"'):
        if type_champ != "Ljava/lang/String;":
            raise ValueError("valeur chaîne sur un champ qui n'est pas String")
        return lire_chaine(texte)
    if type_champ == "Z":
        if texte not in ("true", "false"):
            raise ValueError("booléen true ou false attendu")
        return texte == "true"
    if type_champ in ("F", "D"):
        return float(texte)
    if type_champ in ("B", "S", "C", "I", "J"):
        return int(texte, 0)
    raise ValueError(f"valeur initiale non gérée pour le type {type_champ}")


class Lecteur:
    def __init__(self, fichier):
        self.fichier = fichier
        self.classes = []
        self.classe = None
        self.methode = None
        self.ligne = 0

    def erreur(self, message):
        return ErreurSource(self.fichier, self.ligne, message)

    def lire(self, texte):
        for numero, brute in enumerate(texte.splitlines(), 1):
            self.ligne = numero
            ligne = retirer_commentaire(brute).strip()
            if not ligne:
                continue
            try:
                self.traiter(ligne)
            except ValueError as exc:
                raise self.erreur(str(exc)) from exc
        if self.methode is not None:
            raise self.erreur(f"méthode {self.methode.nom} sans .fin")
        if not self.classes:
            raise self.erreur("aucune classe dans la source")
        return self.classes

    def traiter(self, ligne):
        mot, _, reste = ligne.partition(" ")
        reste = reste.strip()
        if self.methode is not None:
            self.dans_methode(mot, reste, ligne)
        elif mot == ".classe":
            self.ouvrir_classe(reste.split())
        elif self.classe is None:
            raise ValueError(".classe attendu avant toute autre ligne")
        elif mot == ".super":
            self.classe.super = None if reste == "aucune" else self.type_classe(reste)
        elif mot == ".realise":
            self.classe.interfaces.append(self.type_classe(reste))
        elif mot == ".source":
            self.classe.source = lire_chaine(reste)
        elif mot == ".champ":
            self.ajouter_champ(reste)
        elif mot == ".methode":
            self.ouvrir_methode(reste.split())
        else:
            raise ValueError(f"directive inconnue « {mot} »")

    def type_classe(self, texte):
        if not est_type(texte) or texte[0] != "L":
            raise ValueError(f"descripteur de classe invalide « {texte} »")
        return texte

    def ouvrir_classe(self, mots):
        if not mots:
            raise ValueError(".classe sans descripteur")
        descripteur = self.type_classe(mots[-1])
        if any(c.descripteur == descripteur for c in self.classes):
            raise ValueError(f"classe {descripteur} définie deux fois")
        self.classe = Classe(self.ligne, lire_acces(mots[:-1]), descripteur)
        self.classes.append(self.classe)

    def ajouter_champ(self, reste):
        gauche, signe, droite = reste.partition("=")
        mots = gauche.split()
        if not mots or ":" not in mots[-1]:
            raise ValueError(".champ attend [accès] nom:Type [= valeur]")
        nom, _, type_champ = mots[-1].partition(":")
        if not nom or not est_type(type_champ) or type_champ == "V":
            raise ValueError(f"champ invalide « {mots[-1]} »")
        champ = Champ(self.ligne, lire_acces(mots[:-1]), nom, type_champ)
        if signe:
            if not champ.acces & ACCES["static"]:
                raise ValueError("valeur initiale réservée aux champs statiques")
            champ.valeur = (lire_valeur(type_champ, droite.strip()),)
        if any(c.nom == nom and c.type == type_champ for c in self.classe.champs):
            raise ValueError(f"champ {nom} défini deux fois")
        self.classe.champs.append(champ)

    def ouvrir_methode(self, mots):
        trouve = RE_PROTO.match(mots[-1]) if mots else None
        if trouve is None:
            raise ValueError(".methode attend [accès] nom(Paramètres)Retour")
        nom, parametres, retour = trouve.group(1), decouper_types(trouve.group(2)), trouve.group(3)
        if parametres is None or "V" in parametres or not est_type(retour):
            raise ValueError(f"signature invalide « {mots[-1]} »")
        acces = lire_acces(mots[:-1])
        if nom in ("<init>", "<clinit>"):
            acces |= CONSTRUCTEUR
        memes = [m for m in self.classe.methodes if m.nom == nom and m.parametres == parametres]
        if any(m.retour == retour for m in memes):
            raise ValueError(f"méthode {nom} définie deux fois")
        self.methode = Methode(self.ligne, acces, nom, parametres, retour)

    def dans_methode(self, mot, reste, ligne):
        methode = self.methode
        if mot == ".fin":
            self.classe.methodes.append(methode)
            self.methode = None
        elif mot == ".registres":
            methode.registres = int(reste, 0)
            if not 0 <= methode.registres <= 0xFFFF:
                raise ValueError("nombre de registres hors de 0..65535")
        elif mot == ".attrape":
            self.ajouter_essai(reste.split())
        elif mot in (".table-dense", ".table-creuse", ".donnees-tableau"):
            methode.corps.append(Charge(self.ligne, mot[1:], reste.split()))
        elif mot.startswith("."):
            raise ValueError(f"directive inconnue dans une méthode « {mot} »")
        elif mot.startswith(":"):
            if reste or not RE_ETIQUETTE.match(mot):
                raise ValueError(f"étiquette invalide « {ligne} »")
            methode.corps.append(Etiquette(self.ligne, mot[1:]))
        else:
            methode.corps.append(Instruction(self.ligne, mot, decouper_operandes(reste)))

    def ajouter_essai(self, mots):
        if len(mots) != 4 or not all(RE_ETIQUETTE.match(m) for m in mots[1:]):
            raise ValueError(".attrape attend Type|tout :début :fin :gestionnaire")
        type_attrape = None if mots[0] == "tout" else self.type_classe(mots[0])
        etiquettes = [m[1:] for m in mots[1:]]
        self.methode.essais.append(Essai(self.ligne, type_attrape, *etiquettes))


def lire_source(texte, fichier="<source>"):
    return Lecteur(fichier).lire(texte)
