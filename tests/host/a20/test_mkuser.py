import os
import shutil
import subprocess
import sys
import unittest
from pathlib import Path

RACINE = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(RACINE / "tools"))

import mkuser  # noqa: E402

SEL = bytes(range(16))
LIGNE_ATTENDUE = (
    "Utilisateur:10000:000102030405060708090a0b0c0d0e0f:"
    "e418c26f08c4729d239abd46eb0b96554467e23dfb06f2ae503f27e8793af0c0:0\n"
)
OUTIL = RACINE / "tools" / "mkuser.py"


class TestLigne(unittest.TestCase):
    def test_vecteur_connu_mot_de_passe_vide(self):
        self.assertEqual(mkuser.ligne_compte("Utilisateur", 10_000, SEL, b""), LIGNE_ATTENDUE)

    def test_cinq_champs_et_hash_de_32_octets(self):
        champs = mkuser.ligne_compte("Alice", 12_000, SEL, b"secret", 1).rstrip("\n").split(":")
        self.assertEqual(len(champs), 5)
        self.assertEqual(len(bytes.fromhex(champs[3])), 32)
        self.assertEqual(champs[4], "1")

    def test_noms_refuses(self):
        for nom in ["", " a", "a ", "a:b", "a\x01", "a\x7f", "x" * 32, "é" * 16]:
            with self.subTest(nom=nom), self.assertRaises(mkuser.ErreurCompte):
                mkuser.ligne_compte(nom, 10_000, SEL, b"")

    def test_noms_acceptes(self):
        for nom in ["a", "x" * 31, "é" * 15, "Jean Pierre"]:
            with self.subTest(nom=nom):
                mkuser.ligne_compte(nom, 10_000, SEL, b"")

    def test_parametres_refuses(self):
        cas = [
            (9_999, SEL, b"", 0),
            (1_000_001, SEL, b"", 0),
            (10_000, bytes(7), b"", 0),
            (10_000, bytes(33), b"", 0),
            (10_000, SEL, b"x" * 1025, 0),
            (10_000, SEL, b"", 2),
        ]
        for iterations, sel, mdp, drapeaux in cas:
            with self.subTest(iterations=iterations, sel=len(sel), mdp=len(mdp), d=drapeaux):
                with self.assertRaises(mkuser.ErreurCompte):
                    mkuser.ligne_compte("U", iterations, sel, mdp, drapeaux)

    def test_limites_acceptees(self):
        mkuser.ligne_compte("U", 10_000, bytes(8), b"x" * 1024)
        mkuser.ligne_compte("U", 1_000_000, bytes(32), b"", 1)


class TestOutil(unittest.TestCase):
    def setUp(self):
        nom = self.id().rsplit(".", 1)[-1]
        self.dossier = RACINE / "build" / "a20" / "mkuser-test" / nom
        shutil.rmtree(self.dossier, ignore_errors=True)
        self.dossier.mkdir(parents=True)
        self.addCleanup(shutil.rmtree, self.dossier, ignore_errors=True)

    def lancer(self, *args, entree=b""):
        return subprocess.run(
            [sys.executable, str(OUTIL), *args],
            input=entree,
            capture_output=True,
            check=False,
        )

    def test_defauts_vides_et_sel_aleatoire(self):
        a, b = self.dossier / "a" / "users", self.dossier / "b"
        self.assertEqual(self.lancer("--sortie", str(a)).returncode, 0)
        self.assertEqual(self.lancer("--sortie", str(b)).returncode, 0)
        self.assertNotEqual(a.read_text().split(":")[2], b.read_text().split(":")[2])
        self.assertTrue(a.read_text().startswith("Utilisateur:10000:"))
        self.assertEqual(a.stat().st_mode & 0o777, 0o644)
        self.assertEqual([p.name for p in a.parent.iterdir()], ["users"])

    def test_sel_fixe_reproductible(self):
        sortie = self.dossier / "users"
        self.assertEqual(self.lancer("--sortie", str(sortie), "--sel-hex", SEL.hex()).returncode, 0)
        self.assertEqual(sortie.read_text(), LIGNE_ATTENDUE)

    def test_mot_de_passe_sur_stdin(self):
        sortie = self.dossier / "users"
        argv = ["--sortie", str(sortie), "--sel-hex", SEL.hex(), "--mot-de-passe-stdin"]
        self.assertEqual(self.lancer(*argv, entree=b"motdepasse\n").returncode, 0)
        attendu = mkuser.ligne_compte("Utilisateur", 10_000, SEL, b"motdepasse")
        self.assertEqual(sortie.read_text(), attendu)

    def test_erreurs_code_2_sans_fichier(self):
        sortie = self.dossier / "users"
        for argv in (
            ["--compte", "a:b"],
            ["--iterations", "5"],
            ["--sel-hex", "zz"],
            ["--sel-hex", "00"],
        ):
            with self.subTest(argv=argv):
                resultat = self.lancer("--sortie", str(sortie), *argv)
                self.assertEqual(resultat.returncode, 2)
                self.assertFalse(sortie.exists())

    def test_fixture_du_makefile(self):
        brut = os.environ.get("A20_FIXTURE")
        if not brut:
            self.skipTest("A20_FIXTURE non defini (lancer make host-a20)")
        chemin = RACINE / brut
        if not chemin.exists():
            self.skipTest("fixture absente (lancer make host-a20)")
        self.assertEqual(chemin.read_text(), LIGNE_ATTENDUE)


if __name__ == "__main__":
    unittest.main(verbosity=2)
