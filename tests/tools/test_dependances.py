import subprocess
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
B = "build/deps-test"


def variable_make(nom):
    sortie = subprocess.run(
        ["make", "-pn", f"B={B}", "all"],
        cwd=RACINE,
        capture_output=True,
        text=True,
        check=False,
    ).stdout
    for ligne in sortie.splitlines():
        if ligne.startswith(f"{nom} := ") or ligne.startswith(f"{nom} = "):
            return ligne.split("=", 1)[1].split()
    raise AssertionError(f"variable {nom} absente de la base de make")


def test_dependances_des_objets_utilisateur_incluses():
    deps = variable_make("UDEP")
    attendus = (
        "uobj/user/apps/shell/sh_dlg.c.d",
        "uobj/user/libvelum/malloc.c.d",
        "uobj/lib/wm/wmc_conn.c.d",
        "uobj/user/crt/crt0.S.d",
    )
    for suffixe in attendus:
        assert any(d.endswith(suffixe) for d in deps), f"{suffixe} non suivi par make"


def test_une_dependance_par_source_utilisateur():
    deps = variable_make("UDEP")
    sources = variable_make("USRC")
    assert len(sources) > 100
    assert len(deps) == len(sources)


def test_dependances_des_tests_hote_incluses():
    deps = variable_make("HDEP")
    assert len(deps) > 400
    assert any(d.endswith("host/a03/test_reserve.d") for d in deps)
