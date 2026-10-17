import re
import unittest
from pathlib import Path

RACINE = Path(__file__).resolve().parents[3]
DOSSIERS = [
    "common",
    "hello",
    "apkessai",
    "apkinst",
    "apkrun",
    "init",
    "ring3test",
    "spin",
    "vtest",
    "wstress",
]
UICOLOR = RACINE / "user" / "apps" / "common" / "uicolor.h"
LITTERAL_ARGB = re.compile(r"\b0[xX][0-9a-fA-F]{8}[uU]?\b|\b\d{9,10}[uU]\b")
USAGE_UI = re.compile(r"\b(gfx_\w+|t_color|ui_text\w*|luna_\w+|ctl_\w+|FONT_\w+)\b")
IDENT_POLICE = re.compile(r"(?<![A-Za-z0-9_])FONT_[A-Z0-9_]+\b")
DEFINE_UI = re.compile(r"^#\s*define\s+(UI_\w+)\s+(.*)$", re.M)
AUTORISES_UICOLOR = {
    "UI_WHITE": "0xffffffffu",
    "UI_LABEL_BG": "0x80ffffffu",
    "UI_LABEL_BG_D": "0x80141824u",
}


def sans_commentaires(source):
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.S)
    return re.sub(r"//[^\n]*", "", source)


def polices_declarees():
    source = (RACINE / "include" / "velum" / "font.h").read_text(encoding="utf-8")
    corps = re.search(r"typedef enum e_fontid\s*\{(.*?)\}", source, re.S).group(1)
    return re.findall(r"\bFONT_[A-Z0-9_]+\b", corps)


def fichiers():
    for dossier in DOSSIERS:
        base = RACINE / "user" / "apps" / dossier
        yield from sorted(base.glob("*.c"))
        yield from sorted(base.glob("*.h"))


def fichiers_graphiques():
    for chemin in fichiers():
        if chemin == UICOLOR:
            continue
        source = sans_commentaires(chemin.read_text(encoding="utf-8"))
        if USAGE_UI.search(source):
            yield chemin, source


class TestCouleursDesApplications(unittest.TestCase):
    def test_aucune_couleur_en_dur_hors_de_uicolor(self):
        trouvees = []
        vus = 0
        for chemin, source in fichiers_graphiques():
            vus += 1
            for numero, ligne in enumerate(source.splitlines(), 1):
                for litteral in LITTERAL_ARGB.findall(ligne):
                    relatif = chemin.relative_to(RACINE)
                    trouvees.append(f"{relatif}:{numero}: {litteral}")
        self.assertGreater(vus, 4)
        self.assertEqual(trouvees, [], "\n" + "\n".join(trouvees))

    def test_uicolor_ne_definit_que_des_jetons_du_theme(self):
        source = sans_commentaires(UICOLOR.read_text(encoding="utf-8"))
        definitions = dict(DEFINE_UI.findall(source.replace("# define", "#define")))
        self.assertGreater(len(definitions), 20)
        fautes = []
        for nom, valeur in definitions.items():
            valeur = valeur.strip()
            if nom in AUTORISES_UICOLOR:
                if valeur != AUTORISES_UICOLOR[nom]:
                    fautes.append(f"{nom}: {valeur}")
            elif LITTERAL_ARGB.search(valeur):
                fautes.append(f"{nom}: litteral en dur {valeur}")
        self.assertEqual(fautes, [], "\n" + "\n".join(fautes))

    def test_uicolor_inclut_le_theme(self):
        source = UICOLOR.read_text(encoding="utf-8")
        self.assertIn('"velum/theme.h"', source)


class TestPolicesDesApplications(unittest.TestCase):
    def test_enumeration_des_polices_lue(self):
        noms = polices_declarees()
        self.assertEqual(noms[-1], "FONT_IDS")
        self.assertIn("FONT_INS_BOLD_24", noms)
        self.assertEqual(len(noms), len(set(noms)))

    def test_toute_police_utilisee_existe_dans_l_enumeration(self):
        connues = set(polices_declarees()) - {"FONT_IDS"}
        inconnues = []
        vus = 0
        for chemin in fichiers():
            source = sans_commentaires(chemin.read_text(encoding="utf-8"))
            for numero, ligne in enumerate(source.splitlines(), 1):
                for nom in IDENT_POLICE.findall(ligne):
                    vus += 1
                    if nom not in connues:
                        relatif = chemin.relative_to(RACINE)
                        inconnues.append(f"{relatif}:{numero}: {nom}")
        self.assertGreater(vus, 3)
        self.assertEqual(inconnues, [], "\n" + "\n".join(inconnues))
