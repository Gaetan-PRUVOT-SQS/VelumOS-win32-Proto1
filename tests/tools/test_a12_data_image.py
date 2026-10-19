import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
import mkimage  # noqa: E402

SORTIE = RACINE / "build" / "a12" / "tools"
LIMINE = Path(os.environ.get("LIMINE_DIR", RACINE / "build" / "limine"))
OUTILS = ("sgdisk", "mformat", "mmd", "mcopy", "mdir")
CHEMIN_SBIN = "/usr/sbin:/sbin"
IMAGE_POSSIBLE = all(shutil.which(o) for o in OUTILS) and (LIMINE / "limine").is_file()
FSCK = shutil.which("fsck.vfat") or shutil.which("fsck.vfat", path=CHEMIN_SBIN)
OFFSET_ETIQUETTE = 71
OFFSET_TYPE = 82


def test_plan_sans_data_inchange():
    plan = mkimage.planifier(64)
    assert plan.data_taille == 0
    assert plan.data_debut == 0
    assert plan.taille == plan.esp_debut + plan.esp_taille + mkimage.MIO


def test_plan_place_data_apres_l_esp_alignee_sur_1_mio():
    plan = mkimage.planifier(64, 40)
    assert plan.data_debut == plan.esp_debut + plan.esp_taille
    assert plan.data_debut % mkimage.MIO == 0
    assert plan.data_taille == 40 * mkimage.MIO
    assert plan.taille == plan.data_debut + plan.data_taille + mkimage.MIO


@pytest.mark.parametrize("data_mio", [1, 8, 33])
def test_plan_refuse_un_data_trop_petit_pour_fat32(data_mio):
    with pytest.raises(mkimage.Erreur, match="FAT32"):
        mkimage.planifier(64, data_mio)


def test_plan_accepte_la_limite_basse():
    assert mkimage.planifier(8, mkimage.DATA_MIO_MIN).data_taille == 34 * mkimage.MIO


def test_commande_gpt_ajoute_la_partition_data():
    plan = mkimage.planifier(64, 40)
    ident = mkimage.identifiants(1760000000)
    commande = mkimage.commande_gpt(Path("d.img"), plan, ident)
    debut = plan.data_debut // mkimage.SECTEUR
    fin = (plan.data_debut + plan.data_taille) // mkimage.SECTEUR - 1
    assert f"3:{debut}:{fin}" in commande
    assert "3:0700" in commande
    assert "3:data" in commande
    assert f"3:{ident.data}" in commande


def test_commande_gpt_sans_data_garde_deux_partitions():
    commande = mkimage.commande_gpt(Path("d.img"), mkimage.planifier(64), mkimage.identifiants(0))
    assert not any(element.startswith("3:") for element in commande)


def test_identifiants_data_deterministes_et_distincts():
    a, b = mkimage.identifiants(1760000000), mkimage.identifiants(1760000000)
    assert a.data == b.data
    assert a.serie_data == b.serie_data
    assert len({a.disque, a.esp, a.bios, a.data}) == 4
    assert mkimage.identifiants(1760000001).data != a.data


def construire(travail, nom, data_mio):
    noyau = travail / "kernel.elf"
    noyau.write_bytes(b"\x7fELF" + bytes(range(256)) * 40)
    racine = travail / "root"
    (racine / "system").mkdir(parents=True, exist_ok=True)
    (racine / "system" / "os-release").write_bytes(b"VelumOS\n")
    image = travail / nom
    args = mkimage.analyser(
        ["--noyau", str(noyau), "--sortie", str(image), "--initrd", str(racine),
         "--limine", str(LIMINE), "--data-mio", str(data_mio)]
    )  # fmt: skip
    return image, mkimage.fabriquer(args)


@pytest.fixture
def travail():
    dossier = SORTIE / "data_image"
    shutil.rmtree(dossier, ignore_errors=True)
    dossier.mkdir(parents=True)
    return dossier


def extraire(image, plan, sortie):
    with image.open("rb") as source, sortie.open("wb") as cible:
        source.seek(plan.data_debut)
        cible.write(source.read(plan.data_taille))
    return sortie


def verifier_fsck(partition):
    resultat = subprocess.run(
        [FSCK, "-n", str(partition)], capture_output=True, text=True, check=False
    )
    assert resultat.returncode == 0, resultat.stdout + resultat.stderr


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
@pytest.mark.skipif(FSCK is None, reason="fsck.vfat absent")
def test_image_porte_une_partition_data_fat32_propre(travail):
    image, plan = construire(travail, "d.img", 40)
    ident = mkimage.identifiants(mkimage.epoque_source())
    info = subprocess.run(
        ["sgdisk", "-i", "3", str(image)], capture_output=True, text=True, check=True
    ).stdout
    assert "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7" in info
    assert f"Partition unique GUID: {ident.data}" in info
    assert "'data'" in info
    partition = extraire(image, plan, travail / "data.part")
    brut = partition.read_bytes()[:512]
    assert brut[OFFSET_ETIQUETTE : OFFSET_ETIQUETTE + 11] == b"VELUMDATA  "
    assert brut[OFFSET_TYPE : OFFSET_TYPE + 8] == b"FAT32   "
    assert brut[510:512] == b"\x55\xaa"
    verifier_fsck(partition)


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
@pytest.mark.skipif(FSCK is None, reason="fsck.vfat absent")
def test_ecriture_mtools_dans_data_reste_propre_et_relisible(travail):
    image, plan = construire(travail, "e.img", 40)
    cible = f"{image}@@{plan.data_debut}"
    source = travail / "Nom assez long pour un LFN.bin"
    source.write_bytes(bytes((i * 3) % 251 for i in range(100000)))
    subprocess.run(["mmd", "-i", cible, "::/etat"], check=True, capture_output=True)
    subprocess.run(["mcopy", "-i", cible, str(source), "::/etat/"], check=True, capture_output=True)
    relu = subprocess.run(
        ["mcopy", "-n", "-i", cible, f"::/etat/{source.name}", "-"],
        check=True,
        capture_output=True,
    ).stdout
    assert relu == source.read_bytes()
    verifier_fsck(extraire(image, plan, travail / "e.part"))


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
def test_image_sans_data_n_a_que_deux_partitions(travail):
    image, plan = construire(travail, "s.img", 0)
    assert plan.data_taille == 0
    table = subprocess.run(
        ["sgdisk", "-p", str(image)], capture_output=True, text=True, check=True
    ).stdout
    lignes = [ligne.split() for ligne in table.splitlines()]
    numeros = [ligne[0] for ligne in lignes if ligne and ligne[0].isdigit() and len(ligne) > 3]
    assert numeros == ["1", "2"]


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
def test_image_avec_data_reproductible(travail):
    a, _ = construire(travail, "a.img", 40)
    b, _ = construire(travail, "b.img", 40)
    assert a.read_bytes() == b.read_bytes()
