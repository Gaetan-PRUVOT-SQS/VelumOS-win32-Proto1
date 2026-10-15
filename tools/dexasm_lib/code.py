"""Assemblage du corps d'une méthode : adresses, instructions, charges utiles, blocs try."""

import re
import struct
from dataclasses import dataclass

from .opcodes import INSTRUCTIONS, TAILLES
from .source import (
    Charge,
    ErreurSource,
    Etiquette,
    Instruction,
    decouper_types,
    est_type,
    lire_chaine,
)

RE_REF_CHAMP = re.compile(r"^(\[*(?:L[^;\s]+;|[ZBSCIJFD]))->([^:(\s]+):(\S+)$")
RE_REF_METHODE = re.compile(r"^(\[*(?:L[^;\s]+;|[ZBSCIJFD]))->([^(\s]+)\(([^)]*)\)(\S+)$")
RE_REGISTRE = re.compile(r"^([vp])(\d+)$")
GENRES_CHARGE = {
    "packed-switch": "table-dense",
    "sparse-switch": "table-creuse",
    "fill-array-data": "donnees-tableau",
}
SAUT_NUL_INTERDIT = ("10t", "20t", "21t", "22t")


@dataclass
class CodeAssemble:
    registres: int
    ins: int
    outs: int
    unites: list
    essais: list


def mots_parametres(parametres):
    return sum(2 if p in ("J", "D") else 1 for p in parametres)


def lire_ref_champ(texte):
    trouve = RE_REF_CHAMP.match(texte)
    if trouve is None or not est_type(trouve.group(3)):
        raise ValueError(f"référence de champ invalide « {texte} »")
    return trouve.groups()


def lire_ref_methode(texte):
    trouve = RE_REF_METHODE.match(texte)
    parametres = decouper_types(trouve.group(3)) if trouve else None
    if parametres is None or not est_type(trouve.group(4)):
        raise ValueError(f"référence de méthode invalide « {texte} »")
    return trouve.group(1), trouve.group(2), tuple(parametres), trouve.group(4)


def lire_reference(genre, texte):
    if genre == "chaine":
        return lire_chaine(texte)
    if genre == "type":
        if not est_type(texte) or texte == "V":
            raise ValueError(f"type invalide « {texte} »")
        return texte
    if genre == "champ":
        return lire_ref_champ(texte)
    return lire_ref_methode(texte)


def lire_entier(texte):
    propre = texte[:-1] if texte[-1:] in ("L", "l") else texte
    return int(propre, 0)


def lire_litteral(texte, bits, flottant=False):
    hexa = texte.lower().startswith(("0x", "-0x"))
    if flottant and not hexa and texte[-1:] in ("f", "d"):
        if texte[-1] == "f" and bits == 32:
            return struct.unpack("<i", struct.pack("<f", float(texte[:-1])))[0]
        if texte[-1] == "d" and bits == 64:
            return struct.unpack("<q", struct.pack("<d", float(texte[:-1])))[0]
        raise ValueError(f"suffixe flottant incompatible avec {bits} bits « {texte} »")
    valeur = lire_entier(texte)
    haut = (1 << bits) if flottant else (1 << (bits - 1))
    if not -(1 << (bits - 1)) <= valeur < haut:
        raise ValueError(f"littéral {texte} hors de {bits} bits")
    return valeur & ((1 << bits) - 1)


def lire_liste_registres(texte):
    if not (texte.startswith("{") and texte.endswith("}")):
        raise ValueError("liste de registres entre accolades attendue")
    corps = texte[1:-1].strip()
    if ".." in corps:
        debut, _, fin = corps.partition("..")
        return "plage", [debut.strip(), fin.strip()]
    return "liste", [m.strip() for m in corps.split(",")] if corps else []


class Assembleur:
    def __init__(self, fichier, methode, tables):
        self.fichier = fichier
        self.methode = methode
        self.tables = tables
        self.statique = bool(methode.acces & 0x8)
        self.ins = mots_parametres(methode.parametres) + (0 if self.statique else 1)
        self.etiquettes = {}
        self.charges = {}
        self.outs = 0
        self.ligne = methode.ligne

    def erreur(self, message):
        return ErreurSource(self.fichier, self.ligne, message)

    def proteger(self, fonction, *arguments):
        try:
            return fonction(*arguments)
        except ValueError as exc:
            raise self.erreur(str(exc)) from exc

    def collecter(self):
        if self.methode.registres < self.ins:
            raise self.erreur(
                f".registres {self.methode.registres} insuffisant ou absent : "
                f"{self.ins} registre(s) d'arguments"
            )
        for element in self.methode.corps:
            self.ligne = element.ligne
            if not isinstance(element, Instruction):
                continue
            if element.nom not in INSTRUCTIONS:
                raise self.erreur(f"instruction inconnue « {element.nom} »")
            genre = INSTRUCTIONS[element.nom][2]
            if genre:
                if not element.operandes:
                    raise self.erreur(f"{element.nom} : référence manquante")
                reference = self.proteger(lire_reference, genre, element.operandes[-1])
                element.reference = reference
                self.tables.ajouter(genre, reference)
        for essai in self.methode.essais:
            if essai.type is not None:
                self.tables.ajouter("type", essai.type)

    def placer(self):
        adresse = 0
        for element in self.methode.corps:
            self.ligne = element.ligne
            if isinstance(element, Etiquette):
                if element.nom in self.etiquettes:
                    raise self.erreur(f"étiquette :{element.nom} définie deux fois")
                self.etiquettes[element.nom] = adresse
                continue
            if isinstance(element, Charge):
                self.proteger(self.preparer_charge, element)
                bourrage = adresse & 1
                self.deplacer_etiquettes(adresse, bourrage)
                element.adresse = adresse + bourrage
                self.charges[element.adresse] = element
                adresse = element.adresse + element.taille
                continue
            element.adresse = adresse
            adresse += TAILLES[INSTRUCTIONS[element.nom][1]]
        return adresse

    def deplacer_etiquettes(self, adresse, bourrage):
        for nom, valeur in self.etiquettes.items():
            if valeur == adresse and bourrage:
                self.etiquettes[nom] = adresse + bourrage

    def preparer_charge(self, charge):
        arguments = charge.arguments
        if charge.genre == "table-dense":
            if not arguments:
                raise ValueError(".table-dense attend la première clé puis des étiquettes")
            charge.premiere = lire_litteral(arguments[0], 32)
            charge.cibles = [self.nom_etiquette(a) for a in arguments[1:]]
            charge.taille = 4 + 2 * len(charge.cibles)
        elif charge.genre == "table-creuse":
            paires = [a.partition("=") for a in arguments]
            if any(not signe for _, signe, _ in paires):
                raise ValueError(".table-creuse attend des couples clé=:étiquette")
            cles = [lire_entier(cle) for cle, _, _ in paires]
            if len(set(cles)) != len(cles):
                raise ValueError("clé en double dans .table-creuse")
            ordre = sorted(range(len(cles)), key=lambda i: cles[i])
            charge.cles = [lire_litteral(str(cles[i]), 32) for i in ordre]
            charge.cibles = [self.nom_etiquette(paires[i][2]) for i in ordre]
            charge.taille = 2 + 4 * len(cles)
        else:
            self.preparer_donnees(charge)
        if len(getattr(charge, "cibles", ())) > 0xFFFF:
            raise ValueError("table de plus de 65535 cibles")

    def preparer_donnees(self, charge):
        if not charge.arguments or charge.arguments[0] not in ("1", "2", "4", "8"):
            raise ValueError(".donnees-tableau attend une largeur 1, 2, 4 ou 8 puis les valeurs")
        largeur = int(charge.arguments[0])
        charge.largeur = largeur
        valeurs = [lire_litteral(a, 8 * largeur, flottant=True) for a in charge.arguments[1:]]
        charge.octets = b"".join(v.to_bytes(largeur, "little") for v in valeurs)
        charge.nombre = len(valeurs)
        charge.taille = 4 + (len(charge.octets) + 1) // 2

    def nom_etiquette(self, texte):
        if not texte.startswith(":") or len(texte) < 2:
            raise ValueError(f"étiquette attendue, lu « {texte} »")
        return texte[1:]

    def adresse_de(self, texte):
        nom = self.nom_etiquette(texte) if texte.startswith(":") else texte
        if nom not in self.etiquettes:
            raise ValueError(f"étiquette inconnue :{nom}")
        return self.etiquettes[nom]

    def registre(self, texte, bits):
        trouve = RE_REGISTRE.match(texte)
        if trouve is None:
            raise ValueError(f"registre attendu, lu « {texte} »")
        numero = int(trouve.group(2))
        if trouve.group(1) == "p":
            if numero >= self.ins:
                raise ValueError(f"{texte} : la méthode n'a que {self.ins} registre(s) d'arguments")
            numero += self.methode.registres - self.ins
        if numero >= self.methode.registres:
            raise ValueError(f"{texte} hors des {self.methode.registres} registres déclarés")
        if numero >= 1 << bits:
            raise ValueError(f"{texte} ne tient pas sur {bits} bits pour cette instruction")
        return numero

    def saut(self, instruction, texte, bits, fmt):
        ecart = self.adresse_de(texte) - instruction.adresse
        if ecart == 0 and fmt in SAUT_NUL_INTERDIT:
            raise ValueError("saut de déplacement nul interdit")
        if not -(1 << (bits - 1)) <= ecart < 1 << (bits - 1):
            raise ValueError(f"saut de {ecart} unités hors de portée ({bits} bits)")
        return ecart & ((1 << bits) - 1)

    def lier_charge(self, instruction, texte):
        cible = self.adresse_de(texte)
        charge = self.charges.get(cible)
        attendu = GENRES_CHARGE[instruction.nom]
        if charge is None or charge.genre != attendu:
            raise ValueError(f"{instruction.nom} doit désigner une directive .{attendu}")
        if charge.origine not in (-1, instruction.adresse) and attendu != "donnees-tableau":
            raise ValueError("table de switch partagée entre deux instructions")
        charge.origine = instruction.adresse

    def attendre(self, instruction, nombre):
        if len(instruction.operandes) != nombre:
            raise ValueError(
                f"{instruction.nom} attend {nombre} opérande(s), lu {len(instruction.operandes)}"
            )
        return instruction.operandes

    def coder(self, instruction):
        code, fmt, genre = INSTRUCTIONS[instruction.nom]
        indice = self.tables.indice(genre, instruction.reference) if genre else 0
        if genre and fmt != "31c" and indice > 0xFFFF:
            raise ValueError(f"indice {indice} trop grand pour {instruction.nom}")
        return getattr(self, "f" + fmt)(instruction, code, indice)

    def f10x(self, i, code, _):
        self.attendre(i, 0)
        return [code]

    def f12x(self, i, code, _):
        a, b = self.attendre(i, 2)
        return [code | self.registre(a, 4) << 8 | self.registre(b, 4) << 12]

    def f11n(self, i, code, _):
        a, b = self.attendre(i, 2)
        return [code | self.registre(a, 4) << 8 | lire_litteral(b, 4) << 12]

    def f11x(self, i, code, _):
        (a,) = self.attendre(i, 1)
        return [code | self.registre(a, 8) << 8]

    def f10t(self, i, code, _):
        (a,) = self.attendre(i, 1)
        return [code | self.saut(i, a, 8, "10t") << 8]

    def f20t(self, i, code, _):
        (a,) = self.attendre(i, 1)
        return [code, self.saut(i, a, 16, "20t")]

    def f30t(self, i, code, _):
        (a,) = self.attendre(i, 1)
        ecart = self.saut(i, a, 32, "30t")
        return [code, ecart & 0xFFFF, ecart >> 16]

    def f22x(self, i, code, _):
        a, b = self.attendre(i, 2)
        return [code | self.registre(a, 8) << 8, self.registre(b, 16)]

    def f32x(self, i, code, _):
        a, b = self.attendre(i, 2)
        return [code, self.registre(a, 16), self.registre(b, 16)]

    def f21t(self, i, code, _):
        a, b = self.attendre(i, 2)
        return [code | self.registre(a, 8) << 8, self.saut(i, b, 16, "21t")]

    def f21s(self, i, code, _):
        a, b = self.attendre(i, 2)
        return [code | self.registre(a, 8) << 8, lire_litteral(b, 16)]

    def f21h(self, i, code, _):
        a, b = self.attendre(i, 2)
        bits = 64 if "wide" in i.nom else 32
        valeur = lire_litteral(b, bits, flottant=True)
        if valeur & ((1 << (bits - 16)) - 1):
            raise ValueError(f"{i.nom} : seuls les 16 bits de poids fort peuvent être non nuls")
        return [code | self.registre(a, 8) << 8, valeur >> (bits - 16)]

    def f21c(self, i, code, indice):
        a, _ = self.attendre(i, 2)
        return [code | self.registre(a, 8) << 8, indice]

    def f23x(self, i, code, _):
        a, b, c = self.attendre(i, 3)
        return [code | self.registre(a, 8) << 8, self.registre(b, 8) | self.registre(c, 8) << 8]

    def f22b(self, i, code, _):
        a, b, c = self.attendre(i, 3)
        return [code | self.registre(a, 8) << 8, self.registre(b, 8) | lire_litteral(c, 8) << 8]

    def f22t(self, i, code, _):
        a, b, c = self.attendre(i, 3)
        tete = code | self.registre(a, 4) << 8 | self.registre(b, 4) << 12
        return [tete, self.saut(i, c, 16, "22t")]

    def f22s(self, i, code, _):
        a, b, c = self.attendre(i, 3)
        tete = code | self.registre(a, 4) << 8 | self.registre(b, 4) << 12
        return [tete, lire_litteral(c, 16)]

    def f22c(self, i, code, indice):
        a, b, _ = self.attendre(i, 3)
        return [code | self.registre(a, 4) << 8 | self.registre(b, 4) << 12, indice]

    def f31i(self, i, code, _):
        a, b = self.attendre(i, 2)
        if i.nom == "const":
            valeur = lire_litteral(b, 32, flottant=True)
        else:
            valeur = lire_litteral(b, 32)
        return [code | self.registre(a, 8) << 8, valeur & 0xFFFF, valeur >> 16]

    def f31t(self, i, code, _):
        a, b = self.attendre(i, 2)
        self.lier_charge(i, b)
        ecart = self.saut(i, b, 32, "31t")
        return [code | self.registre(a, 8) << 8, ecart & 0xFFFF, ecart >> 16]

    def f31c(self, i, code, indice):
        a, _ = self.attendre(i, 2)
        return [code | self.registre(a, 8) << 8, indice & 0xFFFF, indice >> 16]

    def f51l(self, i, code, _):
        a, b = self.attendre(i, 2)
        valeur = lire_litteral(b, 64, flottant=True)
        return [code | self.registre(a, 8) << 8] + [(valeur >> d) & 0xFFFF for d in (0, 16, 32, 48)]

    def f35c(self, i, code, indice):
        liste, _ = self.attendre(i, 2)
        forme, noms = lire_liste_registres(liste)
        if forme != "liste" or len(noms) > 5:
            raise ValueError(f"{i.nom} attend au plus cinq registres entre accolades")
        registres = [self.registre(n, 4) for n in noms] + [0] * 5
        if i.nom.startswith("invoke"):
            self.outs = max(self.outs, len(noms))
        bas = registres[0] | registres[1] << 4 | registres[2] << 8 | registres[3] << 12
        return [code | registres[4] << 8 | len(noms) << 12, indice, bas]

    def f3rc(self, i, code, indice):
        liste, _ = self.attendre(i, 2)
        forme, noms = lire_liste_registres(liste)
        if forme != "plage":
            raise ValueError(f"{i.nom} attend une plage {{vN .. vM}}")
        premier, dernier = self.registre(noms[0], 16), self.registre(noms[1], 16)
        nombre = dernier - premier + 1
        if not 1 <= nombre <= 255:
            raise ValueError("plage de registres vide ou de plus de 255 registres")
        if i.nom.startswith("invoke"):
            self.outs = max(self.outs, nombre)
        return [code | nombre << 8, indice, premier]

    def coder_charge(self, charge):
        if charge.origine < 0:
            raise ValueError("charge utile qu'aucune instruction ne désigne")
        if charge.genre == "donnees-tableau":
            octets = charge.octets + b"\0" * (len(charge.octets) & 1)
            corps = list(struct.unpack(f"<{len(octets) // 2}H", octets))
            return [0x0300, charge.largeur, charge.nombre & 0xFFFF, charge.nombre >> 16] + corps
        cibles = [(self.adresse_de(c) - charge.origine) & 0xFFFFFFFF for c in charge.cibles]
        if charge.genre == "table-dense":
            mots = [charge.premiere] + cibles
            tete = [0x0100, len(cibles)]
        else:
            mots = charge.cles + cibles
            tete = [0x0200, len(cibles)]
        return tete + [u for m in mots for u in (m & 0xFFFF, m >> 16)]

    def coder_essais(self, taille):
        plages = {}
        for essai in self.methode.essais:
            self.ligne = essai.ligne
            debut = self.proteger(self.adresse_de, essai.debut)
            fin = self.proteger(self.adresse_de, essai.fin)
            cible = self.proteger(self.adresse_de, essai.gestionnaire)
            if not debut < fin <= taille or fin - debut > 0xFFFF or cible >= taille:
                raise self.erreur("bloc .attrape vide, inversé ou hors du code")
            gestionnaires = plages.setdefault((debut, fin), [])
            if gestionnaires and gestionnaires[-1][0] is None:
                raise self.erreur("« tout » doit être le dernier gestionnaire du bloc")
            if any(g[0] == essai.type for g in gestionnaires):
                raise self.erreur("type attrapé deux fois sur le même bloc")
            gestionnaires.append((essai.type, cible))
        ordonnees = sorted(plages.items())
        for (_, fin), ((debut, _), _) in zip(
            [p for p, _ in ordonnees], ordonnees[1:], strict=False
        ):
            if debut < fin:
                raise self.erreur("blocs .attrape qui se chevauchent")
        return [(debut, fin - debut, liste) for (debut, fin), liste in ordonnees]

    def assembler(self):
        taille = self.placer()
        if taille == 0:
            raise self.erreur(f"méthode {self.methode.nom} sans instruction")
        unites = []
        for element in self.methode.corps:
            self.ligne = element.ligne
            if isinstance(element, Instruction):
                unites += self.proteger(self.coder, element)
        position = 0
        sortie = []
        for element in self.methode.corps:
            self.ligne = element.ligne
            if isinstance(element, Instruction):
                longueur = TAILLES[INSTRUCTIONS[element.nom][1]]
                sortie += unites[position : position + longueur]
                position += longueur
            elif isinstance(element, Charge):
                sortie += [0] * (element.adresse - len(sortie))
                sortie += self.proteger(self.coder_charge, element)
        if len(sortie) != taille:
            raise self.erreur("taille de code incohérente (défaut de l'assembleur)")
        essais = self.coder_essais(taille)
        return CodeAssemble(self.methode.registres, self.ins, self.outs, sortie, essais)
