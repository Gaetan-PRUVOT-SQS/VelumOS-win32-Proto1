import re
import unittest
from pathlib import Path

from test_typographie_couleurs import (  # noqa: F401
    TestCouleursDesApplications,
    TestPolicesDesApplications,
)

RACINE = Path(__file__).resolve().parents[3]
DOSSIERS = ["common", "hello", "logon", "shell"]
JETON = re.compile(
    r'(?P<chaine>"(?:[^"\\\n]|\\.)*")'
    r"|(?P<nbsp>\bNBSP\b)"
    r"|(?P<car>'(?:[^'\\\n]|\\.)*')"
    r"|(?P<blanc>\s+)"
    r"|(?P<autre>.)"
)
FAUTE = re.compile(r" [:!?]|« | »|(?:^|\d) (?:j|h|min|s)(?=$|[ .,)])")
INSECABLE = "\u00a0"


def decoder(litteral):
    corps = litteral[1:-1]
    return corps.replace("\\xc2\\xa0", INSECABLE).replace("\\n", "\n")


def textes(source):
    morceaux = []
    ligne = 1
    debut = 0
    for jeton in JETON.finditer(source):
        genre = jeton.lastgroup
        if genre == "chaine":
            if not morceaux:
                debut = ligne
            morceaux.append(decoder(jeton.group()))
        elif genre == "nbsp" and morceaux:
            morceaux.append(INSECABLE)
        elif genre not in ("blanc", "nbsp") and morceaux:
            yield debut, "".join(morceaux)
            morceaux = []
        ligne += jeton.group().count("\n")
    if morceaux:
        yield debut, "".join(morceaux)


def sans_inclusions(source):
    return re.sub(r"(?m)^[ \t]*#[ \t]*include[^\n]*", "", source)


def fautes(texte):
    return [m.group() for m in FAUTE.finditer(texte)]


def fichiers_interface():
    for dossier in DOSSIERS:
        base = RACINE / "user" / "apps" / dossier
        yield from sorted(base.glob("*.c"))
        yield from sorted(base.glob("*.h"))
    yield RACINE / "kernel" / "kcon" / "bsod_text.c"


class TestRegle(unittest.TestCase):
    def test_espace_ordinaire_avant_ponctuation_haute_refusee(self):
        for texte in ["Ouvrir :", "Bonjour !", "Vraiment ?", "a : b"]:
            with self.subTest(texte=texte):
                self.assertTrue(fautes(texte))

    def test_espace_ordinaire_dans_les_guillemets_refusee(self):
        self.assertEqual(fautes("trouver « x »."), ["« ", " »"])

    def test_espace_ordinaire_avant_unite_refusee(self):
        for texte in ["4 min 30 s", "30 s", " s", "1 j 02 h", "dans 5 s."]:
            with self.subTest(texte=texte):
                self.assertTrue(fautes(texte))

    def test_mots_et_unites_insecables_acceptes(self):
        for texte in ["4\u00a0min 30\u00a0s", " j'arrive", " sur", "3 fois", "\u00a0s", " heure"]:
            with self.subTest(texte=texte):
                self.assertEqual(fautes(texte), [])

    def test_espace_insecable_acceptee(self):
        for texte in ["Ouvrir\u00a0:", "Bonjour\u00a0!", "«\u00a0x\u00a0»", "Fait\u00a0?"]:
            with self.subTest(texte=texte):
                self.assertEqual(fautes(texte), [])

    def test_textes_hors_prose_acceptes(self):
        for texte in ["12:30", "/system/bin/hello", "shell: journal", "a?b", "%s:%u", ""]:
            with self.subTest(texte=texte):
                self.assertEqual(fautes(texte), [])


class TestExtraction(unittest.TestCase):
    def test_litteraux_voisins_et_macro_reunis(self):
        source = 'f("Ouvrir" NBSP ":");\ng("a"\n\t"b", \'"\', "c");'
        self.assertEqual(list(textes(source)), [(1, "Ouvrir\u00a0:"), (2, "ab"), (3, "c")])

    def test_echappement_octets_decode(self):
        self.assertEqual(list(textes('"a\\xc2\\xa0" "!"')), [(1, "a\u00a0!")])

    def test_inclusions_ignorees(self):
        self.assertEqual(list(textes(sans_inclusions('#include "a b.h"\n"x"'))), [(2, "x")])


class TestTextesInterface(unittest.TestCase):
    def test_aucune_espace_secable_dans_les_textes_de_l_interface(self):
        trouvees = []
        lus = 0
        for chemin in fichiers_interface():
            source = sans_inclusions(chemin.read_text(encoding="utf-8"))
            for ligne, texte in textes(source):
                lus += 1
                for faute in fautes(texte):
                    relatif = chemin.relative_to(RACINE)
                    trouvees.append(f"{relatif}:{ligne}: {faute!r} dans {texte!r}")
        self.assertGreater(lus, 40)
        self.assertEqual(trouvees, [], "\n" + "\n".join(trouvees))


if __name__ == "__main__":
    unittest.main()
