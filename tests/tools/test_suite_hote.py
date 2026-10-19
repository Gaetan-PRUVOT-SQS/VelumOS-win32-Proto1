import subprocess
import sys
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
OUTIL = RACINE / "tools" / "suite_hote.py"
DOSSIER = RACINE / "build" / "pratiques" / "suite-test"


def ecrire(nom, corps):
    chemin = DOSSIER / nom
    chemin.write_text("#!/usr/bin/env python3\n" + corps, encoding="utf-8")
    chemin.chmod(0o755)
    return chemin


def cli(arguments):
    return subprocess.run(
        [sys.executable, str(OUTIL), *arguments],
        capture_output=True,
        text=True,
        check=False,
    )


@pytest.fixture
def bac():
    if DOSSIER.exists():
        for enfant in DOSSIER.iterdir():
            enfant.unlink()
    DOSSIER.mkdir(parents=True, exist_ok=True)
    yield DOSSIER


def test_tout_vert(bac):
    vert = ecrire("vert", "raise SystemExit(0)\n")
    resultat = cli([str(vert)])
    assert resultat.returncode == 0
    assert f"host {vert}" in resultat.stdout
    assert "suite hôte" not in resultat.stdout


def test_un_echec_ne_coupe_pas_la_suite(bac):
    rouge = ecrire("rouge", "raise SystemExit(1)\n")
    marque = bac / "passe"
    vert = ecrire(
        "vert",
        f"from pathlib import Path\nPath({str(marque)!r}).write_text('1', encoding='utf-8')\n",
    )
    resultat = cli([str(rouge), str(vert)])
    assert resultat.returncode == 1
    assert marque.read_text(encoding="utf-8") == "1"
    assert "suite hôte : 1 échec" in resultat.stdout
    assert str(rouge) in resultat.stdout
    assert str(vert) not in resultat.stdout.split("suite hôte", 1)[1]


def test_deux_echecs(bac):
    un = ecrire("un", "raise SystemExit(2)\n")
    deux = ecrire("deux", "raise SystemExit(3)\n")
    resultat = cli([str(un), str(deux)])
    assert resultat.returncode == 1
    assert "suite hôte : 2 échecs" in resultat.stdout
    assert str(un) in resultat.stdout
    assert str(deux) in resultat.stdout


def test_binaire_absent_puis_la_suite_continue(bac):
    marque = bac / "apres"
    avant = ecrire("avant", "raise SystemExit(0)\n")
    apres = ecrire(
        "apres",
        f"from pathlib import Path\nPath({str(marque)!r}).write_text('1', encoding='utf-8')\n",
    )
    absent = bac / "absent"
    resultat = cli([str(avant), str(absent), str(apres)])
    assert resultat.returncode == 1
    assert marque.is_file()
    assert "suite hôte : 1 échec" in resultat.stdout
    assert str(absent) in resultat.stdout


def test_sans_binaire(bac):
    del bac
    resultat = cli([])
    assert resultat.returncode == 0
    assert resultat.stdout == ""


def test_harnais_branche():
    host = (RACINE / "mk" / "host.mk").read_text(encoding="utf-8")
    assert "tools/suite_hote.py" in host
    assert "|| exit 1" not in host
    assert "-k" in host
    a20 = (RACINE / "mk" / "lot-a20.mk").read_text(encoding="utf-8")
    recette = a20.split("a20-render:", 1)[1].split(".PHONY: a20-typo", 1)[0]
    assert "tools/suite_hote.py" in recette
    assert "|| exit 1" not in recette
