import hashlib
import os
import re
import shutil
import sys
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
import mkimage  # noqa: E402

SORTIE = RACINE / "build" / "repro" / "tests"
MIO = mkimage.MIO


@pytest.fixture
def travail(request):
    dossier = SORTIE / re.sub(r"\W", "_", request.node.name)
    shutil.rmtree(dossier, ignore_errors=True)
    dossier.mkdir(parents=True)
    yield dossier
    shutil.rmtree(dossier, ignore_errors=True)


def ecrire_construction(dossier, noyau=b"ELF-noyau", disque=b"DISQUE" * 100, fichiers=None):
    racine = dossier / "root"
    racine.mkdir(parents=True)
    for nom, contenu in (fichiers or {"system/bin/init": b"init", "system/etc/os": b"v"}).items():
        chemin = racine / nom
        chemin.parent.mkdir(parents=True, exist_ok=True)
        chemin.write_bytes(contenu)
    (dossier / "kernel.elf").write_bytes(noyau)
    (dossier / "disk.img").write_bytes(disque)
    return dossier


def extracteur_fictif(contenu=b"070701initrd"):
    return lambda image: contenu


def test_premier_octet_different_cas_limites():
    assert mkimage.premier_octet_different(b"", b"") is None
    assert mkimage.premier_octet_different(b"abc", b"abc") is None
    assert mkimage.premier_octet_different(b"abc", b"xbc") == 0
    assert mkimage.premier_octet_different(b"abc", b"abx") == 2
    assert mkimage.premier_octet_different(b"abc", b"ab") == 2
    assert mkimage.premier_octet_different(b"ab", b"abc") == 2


@pytest.mark.parametrize("rang", [65535, 65536, 65537, 131071, 200000])
def test_premier_octet_different_aux_frontieres_de_bloc(rang):
    a = bytes(250000)
    b = bytearray(a)
    b[rang] = 1
    assert mkimage.premier_octet_different(a, bytes(b)) == rang


def test_premier_octet_different_donne_le_premier_parmi_plusieurs():
    a = bytes(100000)
    b = bytearray(a)
    b[70000] = 9
    b[80000] = 9
    assert mkimage.premier_octet_different(a, bytes(b)) == 70000


def test_region_image_suit_la_disposition_gpt_fat():
    taille = mkimage.planifier(64).taille
    esp = 2 * MIO
    assert "MBR" in mkimage.region_image(0, taille)
    assert "en-tête GPT" in mkimage.region_image(528, taille)
    assert "entrées GPT" in mkimage.region_image(1024, taille)
    assert "BIOS" in mkimage.region_image(MIO + 10, taille)
    assert "numéro de série" in mkimage.region_image(esp + 39, taille)
    assert "ESP" in mkimage.region_image(esp + 512, taille)
    assert "secours" in mkimage.region_image(taille - 100, taille)


def test_manifeste_ne_depend_ni_de_l_ordre_de_creation_ni_des_dates(travail):
    noms = ["z/fin", "a/debut", "m/milieu", "a/autre"]
    arbres = []
    for nom_arbre, ordre in (("un", noms), ("deux", list(reversed(noms)))):
        racine = travail / nom_arbre
        for nom in ordre:
            chemin = racine / nom
            chemin.parent.mkdir(parents=True, exist_ok=True)
            chemin.write_bytes(nom.encode())
        arbres.append(racine)
    for chemin in arbres[1].rglob("*"):
        os.utime(chemin, (1_000_000_000, 1_000_000_000))
    assert mkimage.manifeste_arbre(arbres[0]) == mkimage.manifeste_arbre(arbres[1])


def test_manifeste_voit_contenu_droits_et_liens(travail):
    racine = travail / "r"
    racine.mkdir()
    (racine / "f").write_bytes(b"x")
    (racine / "f").chmod(0o644)
    de_base = mkimage.manifeste_arbre(racine)
    (racine / "f").write_bytes(b"y")
    assert mkimage.manifeste_arbre(racine) != de_base
    (racine / "f").write_bytes(b"x")
    (racine / "f").chmod(0o755)
    assert mkimage.manifeste_arbre(racine) != de_base
    (racine / "f").chmod(0o644)
    assert mkimage.manifeste_arbre(racine) == de_base
    (racine / "lien").symlink_to("f")
    assert b"lien f lien" in mkimage.manifeste_arbre(racine)


def test_constructions_identiques_sont_declarees_reproductibles(travail):
    a = ecrire_construction(travail / "a")
    b = ecrire_construction(travail / "b")
    elements_a = mkimage.elements_construction(a, extracteur_fictif())
    elements_b = mkimage.elements_construction(b, extracteur_fictif())
    identiques, rapport = mkimage.comparer_elements(elements_a, elements_b)
    assert identiques
    texte = "\n".join(rapport)
    assert texte.count("identique") == len(mkimage.ELEMENTS_COMPARES)
    assert "DIFFERENT" not in texte


def test_rapport_affiche_chaque_empreinte(travail):
    a = ecrire_construction(travail / "a", noyau=b"noyau-A")
    b = ecrire_construction(travail / "b", noyau=b"noyau-B")
    _, rapport = mkimage.comparer_elements(
        mkimage.elements_construction(a, extracteur_fictif()),
        mkimage.elements_construction(b, extracteur_fictif()),
    )
    texte = "\n".join(rapport)
    assert f"kernel.elf   A {hashlib.sha256(b'noyau-A').hexdigest()}" in texte
    assert f"kernel.elf   B {hashlib.sha256(b'noyau-B').hexdigest()}" in texte
    assert f"initrd.cpio  A {hashlib.sha256(b'070701initrd').hexdigest()}" in texte
    assert f"disk.img     A {hashlib.sha256(b'DISQUE' * 100).hexdigest()}" in texte


def test_difference_du_noyau_donne_le_premier_octet(travail):
    a = ecrire_construction(travail / "a", noyau=b"ELF-noyau-un")
    b = ecrire_construction(travail / "b", noyau=b"ELF-noyau-deux")
    identiques, rapport = mkimage.comparer_elements(
        mkimage.elements_construction(a, extracteur_fictif()),
        mkimage.elements_construction(b, extracteur_fictif()),
    )
    assert not identiques
    texte = "\n".join(rapport)
    assert "kernel.elf   DIFFERENT" in texte
    assert "premier octet différent : 10 (0xa), A=0x75 B=0x64" in texte
    assert "rootfs       identique" in texte


def test_difference_de_l_image_donne_la_region(travail):
    disque = bytearray(mkimage.planifier(8).taille)
    modifie = bytearray(disque)
    modifie[528] = 0x55
    a = ecrire_construction(travail / "a", disque=bytes(disque))
    b = ecrire_construction(travail / "b", disque=bytes(modifie))
    identiques, rapport = mkimage.comparer_elements(
        mkimage.elements_construction(a, extracteur_fictif()),
        mkimage.elements_construction(b, extracteur_fictif()),
    )
    assert not identiques
    texte = "\n".join(rapport)
    assert "premier octet différent : 528 (0x210), A=0x00 B=0x55" in texte
    assert "région : en-tête GPT" in texte


def test_difference_de_l_initrd_est_detectee_meme_si_le_reste_est_egal(travail):
    a = ecrire_construction(travail / "a")
    b = ecrire_construction(travail / "b")
    identiques, rapport = mkimage.comparer_elements(
        mkimage.elements_construction(a, extracteur_fictif(b"070701AAAA")),
        mkimage.elements_construction(b, extracteur_fictif(b"070701AAAB")),
    )
    assert not identiques
    texte = "\n".join(rapport)
    assert "initrd.cpio  DIFFERENT" in texte
    assert "premier octet différent : 9" in texte
    assert "kernel.elf   identique" in texte


def test_difference_de_l_arbre_nomme_le_fichier(travail):
    a = ecrire_construction(travail / "a", fichiers={"system/etc/users": b"sel-un", "bin/x": b"x"})
    b = ecrire_construction(
        travail / "b", fichiers={"system/etc/users": b"sel-deux", "bin/x": b"x"}
    )
    identiques, rapport = mkimage.comparer_elements(
        mkimage.elements_construction(a, extracteur_fictif()),
        mkimage.elements_construction(b, extracteur_fictif()),
    )
    assert not identiques
    texte = "\n".join(rapport)
    assert "rootfs       DIFFERENT" in texte
    assert "A seulement : fichier - " in texte
    assert "B seulement : fichier - " in texte
    assert texte.count("system/etc/users") == 2
    assert "bin/x" not in texte


def test_tailles_differentes_signalees(travail):
    a = ecrire_construction(travail / "a", noyau=b"ELF")
    b = ecrire_construction(travail / "b", noyau=b"ELF-plus-long")
    _, rapport = mkimage.comparer_elements(
        mkimage.elements_construction(a, extracteur_fictif()),
        mkimage.elements_construction(b, extracteur_fictif()),
    )
    assert any("tailles différentes : A=3 octets, B=13 octets" in ligne for ligne in rapport)


def test_construction_incomplete_refusee(travail):
    a = ecrire_construction(travail / "a")
    (a / "disk.img").unlink()
    with pytest.raises(mkimage.Erreur, match="disk.img absent"):
        mkimage.elements_construction(a, extracteur_fictif())
    b = ecrire_construction(travail / "b")
    shutil.rmtree(b / "root")
    with pytest.raises(mkimage.Erreur, match="root/ absent"):
        mkimage.elements_construction(b, extracteur_fictif())


def test_ligne_de_commande_code_de_sortie(travail, monkeypatch, capsys):
    monkeypatch.setattr(mkimage, "extraire_initrd", extracteur_fictif())
    a = ecrire_construction(travail / "a")
    b = ecrire_construction(travail / "b")
    assert mkimage.main(["--comparer", str(a), str(b)]) == 0
    assert "reproductible : oui" in capsys.readouterr().out
    (b / "kernel.elf").write_bytes(b"ELF-autre")
    assert mkimage.main(["--comparer", str(a), str(b)]) == 1
    assert "reproductible : NON" in capsys.readouterr().out
    assert mkimage.main(["--comparer", str(a), str(travail / "absent")]) == 1
    assert "absent de la construction" in capsys.readouterr().err


def test_comparer_ne_demande_pas_noyau_ni_sortie(capsys):
    with pytest.raises(SystemExit):
        mkimage.main(["--initrd", "x"])
    assert "--comparer" in capsys.readouterr().err
