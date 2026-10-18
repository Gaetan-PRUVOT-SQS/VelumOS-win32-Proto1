import importlib.util
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
OUTIL = RACINE / "tools" / "drapeaux_noyau.py"


def charger():
    spec = importlib.util.spec_from_file_location("drapeaux_noyau", OUTIL)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def makefile():
    return (RACINE / "Makefile").read_text(encoding="utf-8")


def test_makefile_courant():
    assert charger().verifier(makefile()) == []


def test_retrait_de_werror():
    texte = makefile().replace("-Werror", "-Wno-error", 1)
    manques = charger().verifier(texte)
    assert "KCFLAGS_COMMON sans -Werror" in manques


def test_retrait_du_piege_ubsan():
    texte = makefile().replace("-fsanitize-undefined-trap-on-error", "", 1)
    manques = charger().verifier(texte)
    assert "KCFLAGS_debug sans -fsanitize-undefined-trap-on-error" in manques


def test_retrait_de_ndebug():
    texte = makefile().replace("-DNDEBUG", "", 1)
    manques = charger().verifier(texte)
    assert "KCFLAGS_release sans -DNDEBUG" in manques


def test_composition_sans_profil():
    texte = makefile().replace("$(KCFLAGS_$(PROFIL))", "", 1)
    manques = charger().verifier(texte)
    assert "KCFLAGS sans le profil" in manques


def test_lignes_coupees():
    texte = "KCFLAGS_COMMON := -Wall \\\n\t-Werror\nKCFLAGS_debug := -O1 -fsanitize=undefined \\\n"
    texte += "\t-fsanitize-undefined-trap-on-error -DVELUM_DEBUG\n"
    texte += "KCFLAGS_release := -O2 -DNDEBUG\n"
    texte += "KCFLAGS := $(KCFLAGS_COMMON) $(KCFLAGS_$(PROFIL))\n"
    module = charger()
    manques = [m for m in module.verifier(texte) if m.startswith("KCFLAGS_COMMON sans -Wall")]
    assert manques == []
    assert "KCFLAGS_COMMON sans -Werror" not in module.verifier(texte)
    assert "KCFLAGS_debug sans -DVELUM_DEBUG" not in module.verifier(texte)
