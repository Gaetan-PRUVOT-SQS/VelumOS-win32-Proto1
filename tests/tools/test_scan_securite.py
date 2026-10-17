import importlib.util
import shutil
import subprocess
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
OUTIL = RACINE / "tools" / "scan_securite.py"
DOSSIER = RACINE / "build" / "P15" / "scan-test"


def charger_outil():
    spec = importlib.util.spec_from_file_location("scan_securite", OUTIL)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def ecrire(racine, relatif, contenu="int main(void)\n{\n\treturn (0);\n}\n"):
    chemin = racine / relatif
    chemin.parent.mkdir(parents=True, exist_ok=True)
    chemin.write_text(contenu)


@pytest.fixture
def arbre():
    shutil.rmtree(DOSSIER, ignore_errors=True)
    DOSSIER.mkdir(parents=True)
    yield DOSSIER
    shutil.rmtree(DOSSIER, ignore_errors=True)


def lancer(racine):
    return subprocess.run(
        ["python3", str(OUTIL), "--racine", str(racine)],
        capture_output=True,
        text=True,
        check=False,
    )


def test_hors_depot_liste_les_sources_du_disque(arbre):
    ecrire(arbre, "a.c")
    ecrire(arbre, "sous/b.h")
    ecrire(arbre, "outils/c.py", "print(1)\n")
    liste = sorted(charger_outil().fichiers(arbre))
    assert liste == ["a.c", "outils/c.py", "sous/b.h"]


def test_hors_depot_respecte_le_gitignore_de_base(arbre):
    ecrire(arbre, "a.c")
    ecrire(arbre, "build/x.c")
    ecrire(arbre, "sous/build/y.c")
    ecrire(arbre, "__pycache__/z.py")
    ecrire(arbre, "tools/__pycache__/z.py")
    ecrire(arbre, "m.pyc")
    ecrire(arbre, "journal.log")
    ecrire(arbre, ".pytest_cache/q.py")
    ecrire(arbre, ".ruff_cache/r.py")
    assert charger_outil().fichiers(arbre) == ["a.c"]


def test_hors_depot_exclut_la_maquette_et_les_tiers(arbre):
    ecrire(arbre, "a.c")
    ecrire(arbre, "ux design/velum_theme.h")
    ecrire(arbre, "third_party/lib/x.c")
    ecrire(arbre, "LICENSES/MIT.txt")
    assert charger_outil().fichiers(arbre) == ["a.c"]


def test_hors_depot_sans_git_installe(arbre, monkeypatch):
    ecrire(arbre, "a.c")
    monkeypatch.setenv("PATH", str(arbre / "vide"))
    assert charger_outil().fichiers(arbre) == ["a.c"]


def test_hors_depot_ne_suit_pas_les_liens(arbre):
    ecrire(arbre, "a.c")
    ecrire(arbre, "reel/b.c")
    (arbre / "lien").symlink_to(arbre / "reel", target_is_directory=True)
    liste = charger_outil().fichiers(arbre)
    assert "lien/b.c" not in liste


def test_scan_hors_depot_signale_strcpy_avec_chemin_et_ligne(arbre):
    ecrire(arbre, "src/a.c", 'void f(char *d)\n{\n\tstrcpy(d, "x");\n}\n')
    resultat = lancer(arbre)
    assert resultat.returncode == 1
    assert "src/a.c:3:" in resultat.stdout


def test_scan_hors_depot_propre_rend_zero(arbre):
    ecrire(arbre, "a.c")
    resultat = lancer(arbre)
    assert resultat.returncode == 0
    assert "0 alerte" in resultat.stderr


def test_scan_signale_un_secret(arbre):
    ecrire(arbre, "conf.txt", "cle = AKIA" + "ABCDEFGHIJKLMNOP" + "\n")
    resultat = lancer(arbre)
    assert resultat.returncode == 1
    assert "conf.txt:1:" in resultat.stdout


def test_cyber_ignore_par_chemin_et_ligne(arbre):
    ecrire(arbre, "a.c", 'void f(char *d)\n{\n\tstrcpy(d, "x");\n\tgets(d);\n}\n')
    ecrire(arbre, ".cyber-ignore", "# justifié\na.c:3\n")
    resultat = lancer(arbre)
    assert "a.c:3:" not in resultat.stdout
    assert "a.c:4:" in resultat.stdout


def test_cyber_ignore_par_fichier_et_dossier(arbre):
    ecrire(arbre, "a.c", 'void f(char *d)\n{\n\tstrcpy(d, "x");\n}\n')
    ecrire(arbre, "lib/b.c", "void g(char *d)\n{\n\tgets(d);\n}\n")
    ecrire(arbre, "c.c", 'void h(char *d)\n{\n\tsprintf(d, "x");\n}\n')
    ecrire(arbre, ".cyber-ignore", "a.c\nlib/  # tiers\n")
    resultat = lancer(arbre)
    assert "a.c" not in resultat.stdout
    assert "lib/b.c" not in resultat.stdout
    assert "c.c:3:" in resultat.stdout


def test_cyber_ignore_n_ignore_pas_un_prefixe_de_nom(arbre):
    ecrire(arbre, "lib/b.c", "void g(char *d)\n{\n\tgets(d);\n}\n")
    ecrire(arbre, "libre/d.c", "void g(char *d)\n{\n\tgets(d);\n}\n")
    ecrire(arbre, ".cyber-ignore", "lib/\n")
    resultat = lancer(arbre)
    assert "lib/b.c" not in resultat.stdout
    assert "libre/d.c:3:" in resultat.stdout


def test_depot_git_valide_utilise_git(arbre):
    ecrire(arbre, "suivi.c")
    ecrire(arbre, "ignore.c")
    ecrire(arbre, ".gitignore", "ignore.c\n")
    subprocess.run(["git", "init", "-q"], cwd=arbre, check=True)
    liste = sorted(charger_outil().fichiers(arbre))
    assert liste == [".gitignore", "suivi.c"]
