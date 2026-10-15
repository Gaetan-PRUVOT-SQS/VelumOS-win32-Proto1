import hashlib
import os
import re
import shutil
import subprocess
import sys
import uuid
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
import mkimage  # noqa: E402

SORTIE = RACINE / "build" / "repro" / "tests"
LIMINE = Path(os.environ.get("LIMINE_DIR", RACINE / "build" / "limine"))
OUTILS_IMAGE = ("sgdisk", "mformat", "mmd", "mcopy")
IMAGE_POSSIBLE = all(shutil.which(o) for o in OUTILS_IMAGE) and (LIMINE / "limine").is_file()
HEX128 = re.compile(r"^[0-9a-f]{128}$")


def fabriquer_arbre(racine):
    (racine / "system" / "bin").mkdir(parents=True)
    (racine / "system" / "etc").mkdir()
    (racine / "system" / "bin" / "init").write_bytes(b"\x7fELF" + b"x" * 5)
    (racine / "system" / "bin" / "init").chmod(0o755)
    (racine / "system" / "etc" / "os-release").write_bytes(b"VelumOS\n")
    (racine / "vide").write_bytes(b"")


def lire_cpio(brut):
    entrees, pos = {}, 0
    while True:
        assert brut[pos : pos + 6] == b"070701"
        champs = [int(brut[pos + 6 + 8 * i : pos + 14 + 8 * i], 16) for i in range(13)]
        mode, taille, nom_len = champs[1], champs[6], champs[11]
        debut_nom = pos + 110
        nom = brut[debut_nom : debut_nom + nom_len - 1].decode()
        debut_donnees = (debut_nom + nom_len + 3) & ~3
        if nom == "TRAILER!!!":
            return entrees
        entrees[nom] = (mode, brut[debut_donnees : debut_donnees + taille])
        pos = (debut_donnees + taille + 3) & ~3


def test_cpio_contient_fichiers_et_dossiers(tmp_path):
    fabriquer_arbre(tmp_path)
    entrees = lire_cpio(mkimage.cpio_newc(tmp_path))
    assert set(entrees) == {
        ".", "system", "system/bin", "system/bin/init", "system/etc",
        "system/etc/os-release", "vide",
    }  # fmt: skip
    assert entrees["system/bin/init"][1] == b"\x7fELF" + b"x" * 5
    assert entrees["system/bin/init"][0] == 0o100755
    assert entrees["system/etc/os-release"][0] == 0o100644
    assert entrees["system"][0] == 0o040755
    assert entrees["vide"][1] == b""


def test_cpio_est_deterministe_et_aligne(tmp_path):
    fabriquer_arbre(tmp_path)
    premier = mkimage.cpio_newc(tmp_path)
    assert premier == mkimage.cpio_newc(tmp_path)
    assert len(premier) % 4 == 0


def test_cpio_refuse_les_liens_symboliques(tmp_path):
    fabriquer_arbre(tmp_path)
    (tmp_path / "lien").symlink_to(tmp_path / "vide")
    with pytest.raises(mkimage.Erreur):
        mkimage.cpio_newc(tmp_path)


@pytest.mark.skipif(shutil.which("cpio") is None, reason="cpio absent")
def test_cpio_relu_par_gnu_cpio(tmp_path):
    fabriquer_arbre(tmp_path)
    sortie = subprocess.run(
        ["cpio", "-t", "--quiet"],
        input=mkimage.cpio_newc(tmp_path),
        capture_output=True,
        check=True,
    ).stdout.decode()
    assert "system/bin/init" in sortie.split()


def test_plan_aligne_et_refuse_une_esp_trop_petite():
    plan = mkimage.planifier(64)
    assert plan.esp_debut % mkimage.MIO == 0
    assert plan.taille == plan.esp_debut + plan.esp_taille + mkimage.MIO
    with pytest.raises(mkimage.Erreur):
        mkimage.planifier(4)


def test_config_limine_contient_module_et_resolution():
    texte = mkimage.config_limine("1024x768", "quiet", True)
    assert "module_path: boot():/boot/initrd.cpio" in texte
    assert "resolution: 1024x768x32" in texte
    assert "cmdline: quiet" in texte
    assert "initrd" not in mkimage.config_limine("800x600", "", False)


@pytest.fixture
def travail(request):
    dossier = SORTIE / re.sub(r"\W", "_", request.node.name)
    shutil.rmtree(dossier, ignore_errors=True)
    dossier.mkdir(parents=True)
    yield dossier
    shutil.rmtree(dossier, ignore_errors=True)


def lignes_config(texte):
    return dict(ligne.strip().split(": ", 1) for ligne in texte.splitlines() if ": " in ligne)


def test_blake2b_vecteur_rfc7693():
    attendu = (
        "ba80a53f981c4d0d6a2797b69f12f6e94c212f14685ac4b74b12bb6fdbffa2d1"
        "7d87c5392aab792dc252d5de4533cc9518d38aa8dbf1925ab92386edd4009923"
    )
    assert mkimage.empreinte_blake2b(b"abc") == attendu


def test_config_limine_ecrit_le_blake2b_du_noyau_et_du_module():
    noyau, initrd = b"noyau de test", b"070701initrd"
    hash_noyau = mkimage.empreinte_blake2b(noyau)
    hash_initrd = mkimage.empreinte_blake2b(initrd)
    assert hash_noyau == hashlib.blake2b(noyau, digest_size=64).hexdigest()
    texte = mkimage.config_limine("1024x768", "quiet", True, hash_noyau, hash_initrd)
    champs = lignes_config(texte)
    assert champs["path"] == f"boot():/boot/kernel.elf#{hash_noyau}"
    assert champs["module_path"] == f"boot():/boot/initrd.cpio#{hash_initrd}"
    assert HEX128.match(champs["path"].split("#")[1])
    assert HEX128.match(champs["module_path"].split("#")[1])


def test_config_limine_exige_la_panique_avant_la_premiere_entree():
    texte = mkimage.config_limine("1024x768", "", True, "a" * 128, "b" * 128)
    lignes = texte.splitlines()
    assert lignes.index("hash_mismatch_panic: yes") < lignes.index("/VelumOS")


def test_config_limine_sans_empreinte_reste_possible():
    champs = lignes_config(mkimage.config_limine("800x600", "", True))
    assert champs["path"] == "boot():/boot/kernel.elf"
    assert "#" not in champs["module_path"]


def test_config_limine_sans_initrd_ne_porte_pas_de_module():
    texte = mkimage.config_limine("800x600", "", False, "c" * 128, None)
    assert "module_path" not in texte


def test_fichiers_esp_hache_la_copie_embarquee(travail):
    noyau = travail / "noyau.elf"
    noyau.write_bytes(b"\x7fELF" + bytes(range(256)))
    args = mkimage.analyser(["--noyau", str(noyau), "--sortie", str(travail / "d.img")])
    initrd = b"070701" + b"\0" * 40
    fichiers, empreintes = mkimage.fichiers_esp(travail, args, LIMINE, initrd)
    assert empreintes["/boot/kernel.elf"] == mkimage.empreinte_blake2b(noyau.read_bytes())
    assert empreintes["/boot/initrd.cpio"] == mkimage.empreinte_blake2b(initrd)
    assert fichiers["/boot/kernel.elf"].read_bytes() == noyau.read_bytes()
    assert fichiers["/boot/kernel.elf"] != noyau
    texte = fichiers["/boot/limine/limine.conf"].read_text()
    assert empreintes["/boot/kernel.elf"] in texte
    assert empreintes["/boot/initrd.cpio"] in texte


def test_identifiants_deterministes_et_lies_a_l_epoque():
    a, b = mkimage.identifiants(1760000000), mkimage.identifiants(1760000000)
    assert a == b
    assert a != mkimage.identifiants(1760000002)
    assert len({a.disque, a.esp, a.bios}) == 3
    for guid in (a.disque, a.esp, a.bios):
        assert str(uuid.UUID(guid)).upper() == guid
        assert uuid.UUID(guid).version == 5
    assert re.fullmatch(r"[0-9A-F]{8}", a.serie_fat)


def test_identifiants_epoque_zero_et_valeur_ancree():
    ident = mkimage.identifiants(0)
    assert ident == mkimage.identifiants(mkimage.EPOQUE_DEFAUT)
    assert ident.disque == str(uuid.uuid5(mkimage.ESPACE_IDENTIFIANTS, "0/disque")).upper()


@pytest.mark.parametrize("valeur", ["", "abc", "-1", "1.5", "１２"])
def test_epoque_invalide_refusee(monkeypatch, valeur):
    monkeypatch.setenv("SOURCE_DATE_EPOCH", valeur)
    with pytest.raises(mkimage.Erreur, match="SOURCE_DATE_EPOCH"):
        mkimage.epoque_source()


def test_epoque_absente_vaut_zero(monkeypatch):
    monkeypatch.delenv("SOURCE_DATE_EPOCH", raising=False)
    assert mkimage.epoque_source() == 0


def test_environnement_outils_fixe_fuseau_et_dates_fat():
    env = mkimage.environnement_outils(0)
    assert env["TZ"] == "UTC"
    assert env["LC_ALL"] == "C"
    assert int(env["SOURCE_DATE_EPOCH"]) == mkimage.EPOQUE_FAT_MIN
    assert mkimage.environnement_outils(1760000000)["SOURCE_DATE_EPOCH"] == "1760000000"


def test_commande_gpt_fixe_les_guid():
    plan = mkimage.planifier(64)
    ident = mkimage.identifiants(1760000000)
    commande = mkimage.commande_gpt(Path("d.img"), plan, ident)
    assert commande[commande.index("-U") + 1] == ident.disque
    assert ["-u", f"1:{ident.esp}"] == commande[commande.index("-u") : commande.index("-u") + 2]
    assert f"2:{ident.bios}" in commande
    assert commande.index("-o") < commande.index("-U")


def construire(travail, nom, noyau, initrd, epoque, fuseau):
    os.environ["SOURCE_DATE_EPOCH"] = str(epoque)
    os.environ["TZ"] = fuseau
    image = travail / nom
    args = mkimage.analyser(
        ["--noyau", str(noyau), "--sortie", str(image), "--initrd", str(initrd),
         "--limine", str(LIMINE), "--cmdline", "quiet"]
    )  # fmt: skip
    mkimage.fabriquer(args)
    return image


@pytest.fixture
def arbre_image(travail):
    noyau = travail / "kernel.elf"
    noyau.write_bytes(b"\x7fELF" + bytes(range(256)) * 40)
    racine = travail / "root"
    fabriquer_arbre(racine)
    return noyau, racine


@pytest.fixture
def environnement_restaure():
    avant = {k: os.environ.get(k) for k in ("SOURCE_DATE_EPOCH", "TZ")}
    yield
    for cle, valeur in avant.items():
        if valeur is None:
            os.environ.pop(cle, None)
        else:
            os.environ[cle] = valeur


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
def test_image_identique_malgre_fuseau_et_heure(arbre_image, travail, environnement_restaure):
    noyau, racine = arbre_image
    a = construire(travail, "a.img", noyau, racine, 1760000000, "UTC")
    os.utime(noyau, (1_000_000_000, 1_000_000_000))
    b = construire(travail, "b.img", noyau, racine, 1760000000, "Pacific/Kiritimati")
    assert a.read_bytes() == b.read_bytes()
    c = construire(travail, "c.img", noyau, racine, 1760000002, "UTC")
    assert a.read_bytes() != c.read_bytes()


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
def test_image_porte_les_guid_les_empreintes_et_la_config(
    arbre_image, travail, environnement_restaure
):
    noyau, racine = arbre_image
    image = construire(travail, "g.img", noyau, racine, 1760000000, "UTC")
    ident = mkimage.identifiants(1760000000)
    plan = mkimage.planifier(64)
    env = mkimage.environnement_outils(1760000000)
    disque = subprocess.run(
        ["sgdisk", "-p", str(image)], capture_output=True, text=True, check=True
    )
    assert ident.disque in disque.stdout
    for numero, attendu in ((1, ident.esp), (2, ident.bios)):
        info = subprocess.run(
            ["sgdisk", "-i", str(numero), str(image)], capture_output=True, text=True, check=True
        )
        assert f"Partition unique GUID: {attendu}" in info.stdout
    boot = subprocess.run(
        ["mdir", "-i", f"{image}@@{plan.esp_debut}", "::"], capture_output=True, text=True, env=env
    )
    assert f"{ident.serie_fat[:4]}-{ident.serie_fat[4:]}" in boot.stdout
    conf = mkimage.relire_esp(image, plan, "/boot/limine/limine.conf", env).decode()
    champs = lignes_config(conf)
    relu = mkimage.relire_esp(image, plan, "/boot/kernel.elf", env)
    assert champs["path"].endswith("#" + mkimage.empreinte_blake2b(relu))
    assert relu == noyau.read_bytes()
    initrd = mkimage.relire_esp(image, plan, "/boot/initrd.cpio", env)
    assert initrd == mkimage.cpio_newc(racine)
    assert champs["module_path"].endswith("#" + mkimage.empreinte_blake2b(initrd))


@pytest.mark.skipif(not IMAGE_POSSIBLE, reason="sgdisk, mtools ou build/limine/limine absents")
def test_verification_refuse_un_fichier_different_de_la_config(
    arbre_image, travail, environnement_restaure
):
    noyau, racine = arbre_image
    image = construire(travail, "v.img", noyau, racine, 1760000000, "UTC")
    plan = mkimage.planifier(64)
    env = mkimage.environnement_outils(1760000000)
    juste = mkimage.empreinte_blake2b(noyau.read_bytes())
    mkimage.verifier_empreintes(image, plan, {"/boot/kernel.elf": juste}, env)
    with pytest.raises(mkimage.Erreur, match="empreinte différente"):
        mkimage.verifier_empreintes(image, plan, {"/boot/kernel.elf": "0" * 128}, env)
