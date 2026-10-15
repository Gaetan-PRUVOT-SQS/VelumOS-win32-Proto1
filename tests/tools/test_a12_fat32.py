import importlib.util
from pathlib import Path

import pytest

CHEMIN = Path(__file__).resolve().parent.parent / "qemu" / "a12_fat32.py"
SORTIE_PROPRE = """fsck.fat 4.2 (2021-01-31)
data.img: 6 files, 142/129022 clusters
"""
COMPTEUR_FAUX = "Free cluster summary wrong (128879 vs. really 128880)"
SORTIE_COMPTEUR_FAUX = f"""fsck.fat 4.2 (2021-01-31)
{COMPTEUR_FAUX}
  Auto-correcting.
Leaving filesystem unchanged.
data.img: 6 files, 142/129022 clusters
"""


@pytest.fixture(scope="module")
def scenario():
    spec = importlib.util.spec_from_file_location("a12_fat32", CHEMIN)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_sortie_propre_sans_anomalie(scenario):
    assert scenario.anomalies_fsck(SORTIE_PROPRE) == []


def test_compteur_fsinfo_faux_rapporte(scenario):
    assert scenario.anomalies_fsck(SORTIE_COMPTEUR_FAUX) == [COMPTEUR_FAUX]


@pytest.mark.parametrize(
    "ligne",
    [
        "Dirty bit is set. Fs was not properly unmounted",
        "Cluster 12 is unreadable",
        "/velum/ecrit.txt: File size is 70000 bytes, cluster chain length is 69632 bytes",
        "Orphaned cluster chain",
    ],
)
def test_anomalie_de_structure_detectee(scenario, ligne):
    assert scenario.anomalies_fsck(SORTIE_PROPRE + ligne + "\n") == [ligne]


def test_compteur_faux_et_autre_anomalie_rapportes_ensemble(scenario):
    ligne = "Orphaned cluster chain"
    sortie = SORTIE_COMPTEUR_FAUX + ligne + "\n"
    assert scenario.anomalies_fsck(sortie) == [COMPTEUR_FAUX, ligne]


def test_motif_attendu_est_deterministe_et_non_constant(scenario):
    motif = scenario.motif()
    assert len(motif) == scenario.LONGUEUR_MOTIF
    assert motif == scenario.motif()
    assert len(set(motif)) > 200
    assert motif[:4] == bytes([0, 7, 14, 21])
