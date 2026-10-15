import hashlib
import struct
import sys
import tomllib
import zlib
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
import dexasm  # noqa: E402
from dexasm_lib import ErreurSource, assembler_texte  # noqa: E402
from dexasm_lib.opcodes import INSTRUCTIONS  # noqa: E402

SORTIE = RACINE / "build" / "d06" / "tests"
ESSAIS = RACINE / "user" / "apk" / "essais"
BONJOUR = RACINE / "user" / "apk" / "bonjour" / "Principale.dasm"
SOURCES = sorted(ESSAIS.glob("*.dasm")) + [BONJOUR]
LONGUEURS = (
    (0x00, 0x01, 1), (0x02, 0x02, 2), (0x03, 0x03, 3), (0x04, 0x04, 1), (0x05, 0x05, 2),
    (0x06, 0x06, 3), (0x07, 0x07, 1), (0x08, 0x08, 2), (0x09, 0x09, 3), (0x0A, 0x12, 1),
    (0x13, 0x13, 2), (0x14, 0x14, 3), (0x15, 0x16, 2), (0x17, 0x17, 3), (0x18, 0x18, 5),
    (0x19, 0x1A, 2), (0x1B, 0x1B, 3), (0x1C, 0x1C, 2), (0x1D, 0x1E, 1), (0x1F, 0x20, 2),
    (0x21, 0x21, 1), (0x22, 0x23, 2), (0x24, 0x26, 3), (0x27, 0x28, 1), (0x29, 0x29, 2),
    (0x2A, 0x2C, 3), (0x2D, 0x3D, 2), (0x44, 0x6D, 2), (0x6E, 0x72, 3), (0x74, 0x78, 3),
    (0x7B, 0x8F, 1), (0x90, 0xAF, 2), (0xB0, 0xCF, 1), (0xD0, 0xE2, 2),
)  # fmt: skip
TYPES_CARTE = {0: "entete", 1: "chaines", 2: "types", 3: "protos", 4: "champs", 5: "methodes"}
TYPES_CARTE |= {6: "classes", 0x1000: "carte", 0x1001: "listes", 0x2000: "donnees_classe"}
TYPES_CARTE |= {0x2001: "code", 0x2002: "donnees_chaine", 0x2005: "tableaux"}


def longueur_instruction(code):
    for debut, fin, unites in LONGUEURS:
        if debut <= code <= fin:
            return unites
    raise AssertionError(f"code d'opération inutilisé {code:#x}")


def signe(valeur, bits):
    return valeur - (1 << bits) if valeur >> (bits - 1) else valeur


class Controle:
    def __init__(self, brut):
        self.brut = brut
        self.pos = 0
        self.codes = {}
        self.entete()
        self.carte()
        self.tables()
        self.classes = [self.classe(i) for i in range(self.nombres[5])]

    def u16(self, pos):
        return struct.unpack_from("<H", self.brut, pos)[0]

    def u32(self, pos):
        return struct.unpack_from("<I", self.brut, pos)[0]

    def uleb(self):
        valeur, decalage = 0, 0
        while True:
            octet = self.brut[self.pos]
            self.pos += 1
            valeur |= (octet & 0x7F) << decalage
            decalage += 7
            if not octet & 0x80:
                return valeur

    def sleb(self):
        debut = self.pos
        valeur = self.uleb()
        bits = 7 * (self.pos - debut)
        return signe(valeur, bits)

    def entete(self):
        brut = self.brut
        assert brut[:8] == b"dex\n035\0"
        assert self.u32(8) == zlib.adler32(brut[12:])
        assert brut[12:32] == hashlib.sha1(brut[32:]).digest()
        assert self.u32(32) == len(brut)
        assert self.u32(36) == 0x70 and self.u32(40) == 0x12345678
        assert self.u32(44) == 0 and self.u32(48) == 0
        self.decalage_carte = self.u32(52)
        champs = struct.unpack_from("<12I", brut, 56)
        self.nombres, self.decalages = champs[0::2], champs[1::2]
        taille_donnees, debut_donnees = self.u32(104), self.u32(108)
        assert taille_donnees % 4 == 0 and debut_donnees + taille_donnees == len(brut)
        tailles = (4, 4, 12, 8, 8, 32)
        attendu = 0x70
        for nombre, decalage, taille in zip(self.nombres, self.decalages, tailles, strict=True):
            assert decalage == (attendu if nombre else 0)
            attendu += nombre * taille
        assert debut_donnees == attendu

    def carte(self):
        assert self.decalage_carte % 4 == 0
        nombre = self.u32(self.decalage_carte)
        entrees = [
            struct.unpack_from("<HHII", self.brut, self.decalage_carte + 4 + 12 * i)
            for i in range(nombre)
        ]
        assert self.decalage_carte + 4 + 12 * nombre == len(self.brut)
        decalages = [e[3] for e in entrees]
        assert decalages == sorted(decalages) and len(set(decalages)) == nombre
        self.sections = {TYPES_CARTE[e[0]]: (e[2], e[3]) for e in entrees}
        assert len(self.sections) == nombre
        assert self.sections["entete"] == (1, 0)
        assert self.sections["carte"] == (1, self.decalage_carte)
        noms = ("chaines", "types", "protos", "champs", "methodes", "classes")
        for nom, nombre, decalage in zip(noms, self.nombres, self.decalages, strict=True):
            assert self.sections.get(nom, (0, 0)) == (nombre, decalage)
        assert self.sections["donnees_chaine"][0] == self.nombres[0]

    def chaine(self, indice):
        self.pos = self.u32(self.decalages[0] + 4 * indice)
        longueur = self.uleb()
        fin = self.brut.index(b"\0", self.pos)
        brut = self.brut[self.pos : fin].replace(b"\xc0\x80", b"\0")
        texte = brut.decode("utf-8", "surrogatepass")
        assert len(texte.encode("utf-16-le", "surrogatepass")) // 2 == longueur
        return texte

    def liste_types(self, decalage):
        if decalage == 0:
            return ()
        assert decalage % 4 == 0
        nombre = self.u32(decalage)
        return struct.unpack_from(f"<{nombre}H", self.brut, decalage + 4)

    def tables(self):
        n = self.nombres
        self.chaines = [self.chaine(i) for i in range(n[0])]
        cles = [list(s.encode("utf-16-be", "surrogatepass")) for s in self.chaines]
        assert all(a < b for a, b in zip(cles, cles[1:], strict=False))
        self.types = [self.u32(self.decalages[1] + 4 * i) for i in range(n[1])]
        assert all(a < b for a, b in zip(self.types, self.types[1:], strict=False))
        assert all(t < n[0] for t in self.types)
        self.protos = []
        for i in range(n[2]):
            court, retour, liste = struct.unpack_from("<III", self.brut, self.decalages[2] + 12 * i)
            parametres = self.liste_types(liste)
            assert retour < n[1] and all(p < n[1] for p in parametres)
            noms = [self.nom_type(t) for t in (retour, *parametres)]
            assert self.chaines[court] == "".join("L" if t[0] in "L[" else t for t in noms)
            self.protos.append((retour, list(parametres)))
        assert all(a < b for a, b in zip(self.protos, self.protos[1:], strict=False))
        self.champs = self.membres_tables(3, n[1])
        self.methodes = self.membres_tables(4, n[2])

    def membres_tables(self, rang, limite):
        elements = []
        for i in range(self.nombres[rang]):
            classe, second, nom = struct.unpack_from(
                "<HHI", self.brut, self.decalages[rang] + 8 * i
            )
            assert classe < self.nombres[1] and second < limite and nom < self.nombres[0]
            elements.append((classe, nom, second))
        assert all(a < b for a, b in zip(elements, elements[1:], strict=False))
        return elements

    def nom_type(self, indice):
        return self.chaines[self.types[indice]]

    def classe(self, rang):
        champs = struct.unpack_from("<8I", self.brut, self.decalages[5] + 32 * rang)
        indice, acces, parent, interfaces, source, annotations, donnees, statiques = champs
        assert indice < self.nombres[1] and annotations == 0
        assert parent == 0xFFFFFFFF or parent < self.nombres[1]
        assert source == 0xFFFFFFFF or source < self.nombres[0]
        definie = {
            "nom": self.nom_type(indice),
            "acces": acces,
            "parent": None if parent == 0xFFFFFFFF else self.nom_type(parent),
            "interfaces": [self.nom_type(t) for t in self.liste_types(interfaces)],
            "champs": [],
            "methodes": {},
            "statiques": statiques,
        }
        if donnees:
            self.donnees_classe(definie, indice, donnees)
        return definie

    def donnees_classe(self, definie, indice_classe, decalage):
        self.pos = decalage
        tailles = [self.uleb() for _ in range(4)]
        for groupe in range(2):
            indice = 0
            for rang in range(tailles[groupe]):
                ecart = self.uleb()
                assert rang == 0 or ecart > 0
                indice += ecart
                acces = self.uleb()
                assert self.champs[indice][0] == indice_classe
                assert bool(acces & 0x8) == (groupe == 0)
                definie["champs"].append((self.chaines[self.champs[indice][1]], acces))
        for groupe in range(2):
            indice = 0
            for rang in range(tailles[2 + groupe]):
                ecart = self.uleb()
                assert rang == 0 or ecart > 0
                indice += ecart
                acces, code = self.uleb(), self.uleb()
                classe, nom, proto = self.methodes[indice]
                assert classe == indice_classe
                assert bool(acces & 0x1000A) == (groupe == 0)
                assert bool(code) != bool(acces & 0x500)
                suite = self.pos
                if code:
                    self.code(code, proto, bool(acces & 0x8))
                self.pos = suite
                definie["methodes"][self.chaines[nom]] = (acces, code)

    def code(self, decalage, proto, statique):
        assert decalage % 4 == 0
        registres, ins, outs, essais, debogage, taille = struct.unpack_from(
            "<4HII", self.brut, decalage
        )
        retour, parametres = self.protos[proto]
        mots = sum(2 if self.nom_type(p) in ("J", "D") else 1 for p in parametres)
        assert ins == mots + (0 if statique else 1) and registres >= ins and debogage == 0
        base = decalage + 16
        assert base + 2 * taille <= len(self.brut)
        unites = struct.unpack_from(f"<{taille}H", self.brut, base)
        debuts, sauts, charges, appels, pc = set(), [], [], 0, 0
        while pc < taille:
            mot = unites[pc]
            if mot in (0x0100, 0x0200, 0x0300):
                assert pc % 2 == 0
                charges.append(pc)
                pc += self.longueur_charge(unites, pc)
                continue
            code = mot & 0xFF
            debuts.add(pc)
            longueur = longueur_instruction(code)
            assert pc + longueur <= taille
            self.relever(unites, pc, sauts)
            if 0x6E <= code <= 0x72:
                appels = max(appels, mot >> 12)
            elif 0x74 <= code <= 0x78:
                appels = max(appels, mot >> 8)
            pc += longueur
        assert pc == taille and outs == appels
        for origine, cible, genre in sauts:
            if genre == "saut":
                assert cible in debuts, f"saut de {origine} vers {cible}"
            else:
                assert cible in charges
                assert unites[cible] == genre
                for ecart in self.cibles_charge(unites, cible):
                    assert origine + ecart in debuts
        self.codes[decalage] = (registres, ins, outs, unites)
        self.essais(base + 2 * taille, essais, taille, debuts)

    def longueur_charge(self, unites, pc):
        nombre = unites[pc + 1]
        if unites[pc] == 0x0100:
            return 4 + 2 * nombre
        if unites[pc] == 0x0200:
            return 2 + 4 * nombre
        elements = unites[pc + 2] | unites[pc + 3] << 16
        assert nombre in (1, 2, 4, 8)
        return 4 + (elements * nombre + 1) // 2

    def cibles_charge(self, unites, pc):
        nombre = unites[pc + 1]
        if unites[pc] == 0x0300:
            return []
        debut = pc + 4 if unites[pc] == 0x0100 else pc + 2 + 2 * nombre
        if unites[pc] == 0x0200:
            cles = [signe(unites[pc + 2 + 2 * i] | unites[pc + 3 + 2 * i] << 16, 32)
                    for i in range(nombre)]  # fmt: skip
            assert all(a < b for a, b in zip(cles, cles[1:], strict=False))
        return [signe(unites[debut + 2 * i] | unites[debut + 2 * i + 1] << 16, 32)
                for i in range(nombre)]  # fmt: skip

    def relever(self, unites, pc, sauts):
        code = unites[pc] & 0xFF
        if code == 0x28:
            sauts.append((pc, pc + signe(unites[pc] >> 8, 8), "saut"))
        elif code == 0x29 or 0x32 <= code <= 0x3D:
            sauts.append((pc, pc + signe(unites[pc + 1], 16), "saut"))
        elif code == 0x2A:
            sauts.append((pc, pc + signe(unites[pc + 1] | unites[pc + 2] << 16, 32), "saut"))
        elif code in (0x26, 0x2B, 0x2C):
            genre = {0x26: 0x0300, 0x2B: 0x0100, 0x2C: 0x0200}[code]
            sauts.append((pc, pc + signe(unites[pc + 1] | unites[pc + 2] << 16, 32), genre))

    def essais(self, fin_code, nombre, taille, debuts):
        if nombre == 0:
            return
        debut = (fin_code + 3) & ~3
        liste = debut + 8 * nombre
        precedent = 0
        for rang in range(nombre):
            adresse, compte, gestionnaire = struct.unpack_from("<IHH", self.brut, debut + 8 * rang)
            assert precedent <= adresse and compte > 0 and adresse + compte <= taille
            assert adresse in debuts
            precedent = adresse + compte
            self.pos = liste + gestionnaire
            types = self.sleb()
            for _ in range(abs(types)):
                assert self.uleb() < self.nombres[1]
                assert self.uleb() in debuts
            if types <= 0:
                assert self.uleb() in debuts


def assembler_source(chemin):
    return dexasm.assembler_fichiers([chemin])


def unites_de(texte, registres=16, signature="m()V", acces="public static"):
    source = f".classe LT;\n.methode {acces} {signature}\n.registres {registres}\n{texte}\n.fin\n"
    controle = Controle(assembler_texte(source))
    (code,) = controle.codes.values()
    return list(code[3])


@pytest.mark.parametrize("chemin", SOURCES, ids=lambda c: c.stem)
def test_structure_chaque_exemple_se_relit(chemin):
    controle = Controle(assembler_source(chemin))
    assert controle.classes and controle.codes


def test_structure_tous_les_essais_dans_un_seul_fichier():
    SORTIE.mkdir(parents=True, exist_ok=True)
    cible = SORTIE / "essais.dex"
    arguments = [str(c) for c in sorted(ESSAIS.glob("*.dasm"))] + ["--sortie", str(cible)]
    assert dexasm.main(arguments) == 0
    controle = Controle(cible.read_bytes())
    noms = [c["nom"] for c in controle.classes]
    assert noms.index("Lcom/velum/essais/Forme;") < noms.index("Lcom/velum/essais/Carre;")
    assert noms.index("Lcom/velum/essais/Carre;") < noms.index("Lcom/velum/essais/Cube;")


def test_resultats_chaque_essai_a_sa_methode_calcul():
    resultats = tomllib.loads((ESSAIS / "resultats.toml").read_text(encoding="utf-8"))
    assert len(resultats) == len(list(ESSAIS.glob("*.dasm")))
    for essai in resultats.values():
        controle = Controle(assembler_source(ESSAIS / essai["source"]))
        classe = next(c for c in controle.classes if c["nom"] == essai["classe"])
        acces, code = classe["methodes"]["calcul"]
        assert acces & 0x9 == 0x9 and code
        assert -(1 << 31) <= essai["resultat"] < 1 << 31


def test_bonjour_classe_champs_methodes_et_chaines():
    controle = Controle(assembler_source(BONJOUR))
    (classe,) = controle.classes
    assert classe["nom"] == "Lcom/velum/bonjour/Principale;"
    assert classe["parent"] == "Landroid/app/Activity;"
    assert classe["interfaces"] == ["Landroid/view/View$OnClickListener;"]
    assert {nom for nom, _ in classe["champs"]} == {"compteur", "texte"}
    assert set(classe["methodes"]) == {"<init>", "onCreate", "onClick"}
    assert classe["methodes"]["<init>"][0] & 0x10000
    for texte in ("Bonjour depuis un APK", "Compter", "Clics : ", "setContentView"):
        assert texte in controle.chaines


def test_codage_instructions_connues_de_la_specification():
    assert unites_de("const/4 v0, -1\nreturn-void") == [0xF012, 0x000E]
    assert unites_de("add-int v0, v1, v2\nreturn-void")[:2] == [0x0090, 0x0201]
    assert unites_de("move/from16 v3, v15\nreturn-void")[:2] == [0x0302, 0x000F]
    assert unites_de("const-wide/high16 v0, 0x4000000000000000\nreturn-void")[:2] == [
        0x0019,
        0x4000,
    ]
    assert unites_de("const v1, 0x12345678\nreturn-void")[:3] == [0x0114, 0x5678, 0x1234]
    assert unites_de("const-wide v2, -2\nreturn-void")[:5] == [0x0218] + [0xFFFE] + [0xFFFF] * 3
    assert unites_de("add-int/lit8 v1, v2, -3\nreturn-void")[:2] == [0x01D8, 0xFD02]
    assert unites_de("if-eq v1, v2, :a\n:a\nreturn-void")[:2] == [0x2132, 0x0002]
    assert unites_de("const v0, 1.0f\nreturn-void")[:3] == [0x0014, 0x0000, 0x3F80]
    appel = unites_de("invoke-static {v1, v2, v3}, LT;->f(III)V\nreturn-void")
    assert appel[0] == 0x3071 and appel[2] == 0x0321
    plage = unites_de("invoke-static/range {v4 .. v9}, LT;->g(IIIIII)V\nreturn-void")
    assert plage[0] == 0x0677 and plage[2] == 4


def test_codage_registres_p_places_en_fin_de_cadre():
    unites = unites_de("move v0, p1\nreturn-void", 5, "m(I)V", "public")
    assert unites[0] == 0x4001


def test_codage_charges_alignees_et_cibles_relatives():
    texte = (
        "const/4 v0, 1\npacked-switch v0, :t\n:a\nreturn-void\n:b\nreturn-void\n"
        ":t\n.table-dense 1 :a :b\n"
    )
    unites = unites_de(texte)
    assert unites[1:4] == [0x002B, 5, 0]
    assert unites[4:6] == [0x000E, 0x000E]
    assert unites[6:] == [0x0100, 2, 1, 0, 3, 0, 4, 0]
    creuse = unites_de(
        "const/4 v0, 1\nsparse-switch v0, :t\n:a\nreturn-void\n:t\n.table-creuse 9=:a -2=:a\n"
    )
    assert creuse[5] == 0 and creuse[6:8] == [0x0200, 2]
    assert creuse[8:12] == [0xFFFE, 0xFFFF, 9, 0]
    donnees = unites_de(
        "const/4 v0, 3\nnew-array v1, v0, [B\nfill-array-data v1, :d\nreturn-void\n"
        ":d\n.donnees-tableau 1 1 2 -1\n"
    )
    assert donnees[-6:] == [0x0300, 1, 3, 0, 0x0201, 0x00FF]


def test_codage_toutes_les_instructions_s_assemblent():
    operandes = {
        "10x": "", "12x": "v0, v1", "11n": "v0, 1", "11x": "v0", "10t": ":fin", "20t": ":fin",
        "30t": ":fin", "22x": "v0, v1", "32x": "v0, v1", "21t": "v0, :fin", "21s": "v0, 1",
        "21h": "v0, 0", "23x": "v0, v1, v2", "22b": "v0, v1, 1", "22t": "v0, v1, :fin",
        "22s": "v0, v1, 1", "31i": "v0, 1", "51l": "v0, 1",
    }  # fmt: skip
    references = {
        "chaine": '"x"', "type": "[I", "champ": "LT;->c:I", "methode": "LT;->f(I)V",
    }  # fmt: skip
    charges = {"fill-array-data": ":d", "packed-switch": ":p", "sparse-switch": ":s"}
    lignes, codes = [], []
    for nom, (code, fmt, genre) in sorted(INSTRUCTIONS.items(), key=lambda e: e[1][0]):
        if nom in charges:
            texte = f"v0, {charges[nom]}"
        elif fmt == "35c":
            texte = "{v0}, " + references[genre]
        elif fmt == "3rc":
            texte = "{v0 .. v0}, " + references[genre]
        elif genre:
            texte = {"21c": "v0, ", "31c": "v0, ", "22c": "v0, v1, "}[fmt] + references[genre]
        else:
            texte = operandes[fmt]
        lignes.append(f"{nom} {texte}")
        codes.append(code)
        if fmt == "10t":
            lignes[-1:] = ["goto :proche", ":proche"]
    lignes += [":fin", "return-void", ":d", ".donnees-tableau 4 1", ":p", ".table-dense 0 :fin"]
    lignes += [":s", ".table-creuse 5=:fin"]
    unites = unites_de("\n".join(lignes))
    relus, pc = [], 0
    while unites[pc] not in (0x0100, 0x0200, 0x0300) and len(relus) < len(codes):
        relus.append(unites[pc] & 0xFF)
        pc += longueur_instruction(unites[pc] & 0xFF)
    assert relus == codes
    assert set(codes) == set(range(0xE3)) - set(range(0x3E, 0x44)) - {0x73, 0x79, 0x7A}


def test_valeurs_statiques_codees_dans_l_ordre_des_champs():
    source = (
        '.classe LT;\n.champ static a:I = -2\n.champ static b:Ljava/lang/String; = "é"\n'
        ".champ static c:Z = true\n.champ static d:J = 0x1ff\n.champ static e:I\n"
    )
    brut = assembler_texte(source)
    controle = Controle(brut)
    debut = controle.classes[0]["statiques"]
    indice = controle.chaines.index("é")
    attendu = bytes((4, 0x04, 0xFE, 0x17, indice, 0x3F, 0x26, 0xFF, 0x01))
    assert brut[debut : debut + len(attendu)] == attendu
    assert controle.sections["tableaux"] == (1, debut)


def test_chaines_triees_par_unites_utf16_et_mutf8():
    source = '.classe LT;\n.champ static a:Ljava/lang/String; = "\\u0000z\\ud83d\\ude00"\n'
    source += '.champ static b:Ljava/lang/String; = "\\uffff"\n'
    controle = Controle(assembler_texte(source))
    assert "\0z\ud83d\ude00" in controle.chaines


ERREURS = {
    "instruction_inconnue": (".methode static m()V\n.registres 1\nfoo v0\n.fin", 4, "inconnue"),
    "etiquette_inconnue": (".methode static m()V\n.registres 1\ngoto :x\n.fin", 4, ":x"),
    "etiquette_double": (
        ".methode static m()V\n.registres 1\n:a\nnop\n:a\nreturn-void\n.fin",
        6,
        "deux fois",
    ),
    "registre_hors_cadre": (".methode static m()V\n.registres 1\nconst/4 v1, 0\n.fin", 4, "v1"),
    "registre_trop_large": (".methode static m()V\n.registres 20\nconst/4 v16, 0\n.fin", 4, "4"),
    "litteral_trop_grand": (".methode static m()V\n.registres 1\nconst/4 v0, 8\n.fin", 4, "8"),
    "registres_absents": (".methode static m(I)V\nreturn-void\n.fin", 2, ".registres"),
    "methode_sans_fin": (".methode static m()V\n.registres 1\nreturn-void", 4, ".fin"),
    "directive_inconnue": (".truc", 2, "inconnue"),
    "operande_manquant": (".methode static m()V\n.registres 2\nmove v0\n.fin", 4, "opérande"),
    "saut_nul": (".methode static m()V\n.registres 1\n:a\ngoto :a\n.fin", 5, "nul"),
    "charge_du_mauvais_genre": (
        ".methode static m()V\n.registres 1\npacked-switch v0, :t\nreturn-void\n"
        ":t\n.table-creuse 1=:t\n.fin",
        4,
        "table-dense",
    ),
    "attrape_inverse": (
        ".methode static m()V\n.registres 1\n:a\nnop\n:b\nreturn-void\n"
        ".attrape tout :b :a :a\n.fin",
        8,
        "attrape",
    ),
    "valeur_sur_champ_d_instance": (".champ x:I = 3", 2, "statiques"),
    "reference_de_methode_invalide": (
        ".methode static m()V\n.registres 1\ninvoke-static {}, LT;->f\n.fin",
        4,
        "méthode",
    ),
}


@pytest.mark.parametrize("nom", sorted(ERREURS))
def test_erreur_de_source_signalee_avec_sa_ligne(nom):
    corps, ligne, fragment = ERREURS[nom]
    with pytest.raises(ErreurSource) as info:
        assembler_texte(".classe LT;\n" + corps + "\n", "essai.dasm")
    assert info.value.ligne == ligne
    assert fragment in info.value.message
    assert str(info.value).startswith(f"essai.dasm:{ligne}: ")


def test_erreur_saut_court_hors_de_portee():
    corps = "goto :loin\n" + "nop\n" * 200 + ":loin\nreturn-void"
    with pytest.raises(ErreurSource) as info:
        unites_de(corps)
    assert info.value.ligne == 4 and "portée" in info.value.message


def test_erreur_en_ligne_de_commande_code_de_sortie_et_message(capsys):
    SORTIE.mkdir(parents=True, exist_ok=True)
    source = SORTIE / "faux.dasm"
    source.write_text(".classe LT;\n.methode static m()V\n.registres 1\nbof\n.fin\n")
    cible = SORTIE / "faux.dex"
    cible.unlink(missing_ok=True)
    assert dexasm.main([str(source), "--sortie", str(cible)]) == 1
    assert f"{source}:4: " in capsys.readouterr().err
    assert not cible.exists()
    assert dexasm.main([str(SORTIE / "absent.dasm"), "--sortie", str(cible)]) == 1
