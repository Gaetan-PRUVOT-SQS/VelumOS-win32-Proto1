import hashlib
import io
import struct
import subprocess
import sys
import zipfile
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
import dexasm  # noqa: E402
import mkapk  # noqa: E402

SORTIE = RACINE / "build" / "d06" / "tests"
CLES = SORTIE / "cles"
BONJOUR = RACINE / "user" / "apk" / "bonjour"
ID_V2 = 0x7109871A


class EchecSignature(Exception):
    pass


@pytest.fixture(scope="module")
def dex():
    return dexasm.assembler_fichiers([BONJOUR / "Principale.dasm"])


@pytest.fixture(scope="module")
def description():
    return mkapk.lire_description((BONJOUR / "bonjour.toml").read_text(encoding="utf-8"))


@pytest.fixture(scope="module")
def apk(dex, description):
    SORTIE.mkdir(parents=True, exist_ok=True)
    return mkapk.fabriquer(description, dex, CLES)


def lire_reserve(brut, debut):
    genre, entete, taille, nombre, _, drapeaux, chaines, _ = struct.unpack_from(
        "<HHI5I", brut, debut
    )
    assert genre == 0x0001 and entete == 28 and taille % 4 == 0
    textes = []
    for rang in range(nombre):
        pos = debut + chaines + struct.unpack_from("<I", brut, debut + 28 + 4 * rang)[0]
        if drapeaux & 0x100:
            unites, octets = brut[pos], brut[pos + 1]
            assert unites < 0x80 and octets < 0x80 and brut[pos + 2 + octets] == 0
            textes.append(brut[pos + 2 : pos + 2 + octets].decode("utf-8"))
        else:
            unites = struct.unpack_from("<H", brut, pos)[0]
            fin = pos + 2 + 2 * unites
            assert brut[fin : fin + 2] == b"\0\0"
            textes.append(brut[pos + 2 : fin].decode("utf-16-le"))
    return textes, debut + taille


def lire_manifeste(brut):
    genre, entete, taille = struct.unpack_from("<HHI", brut, 0)
    assert (genre, entete, taille) == (0x0003, 8, len(brut))
    chaines, pos = lire_reserve(brut, 8)
    elements, identifiants, ouverts, espaces = [], [], [], 0
    while pos < len(brut):
        genre, entete, taille = struct.unpack_from("<HHI", brut, pos)
        assert taille >= entete and pos + taille <= len(brut)
        if genre == 0x0180:
            identifiants = list(struct.unpack_from(f"<{(taille - 8) // 4}I", brut, pos + 8))
        elif genre in (0x0100, 0x0101):
            assert taille == 24
            espaces += 1 if genre == 0x0100 else -1
        elif genre == 0x0102:
            _, nom, debut, pas, nombre = struct.unpack_from("<IIHHH", brut, pos + 16)
            assert (debut, pas) == (20, 20) and taille == 36 + 20 * nombre
            attributs = {}
            for rang in range(nombre):
                espace, cle, texte, _, _, type_valeur, donnee = struct.unpack_from(
                    "<IIIHBBI", brut, pos + 36 + 20 * rang
                )
                identifiant = identifiants[cle] if cle < len(identifiants) else 0
                assert (espace != 0xFFFFFFFF) == (identifiant != 0)
                if type_valeur == 0x03:
                    assert texte == donnee
                    donnee = chaines[donnee]
                attributs[chaines[cle]] = (identifiant, type_valeur, donnee)
            elements.append(("/".join([*ouverts, chaines[nom]]), attributs))
            ouverts.append(chaines[nom])
        elif genre == 0x0103:
            assert chaines[struct.unpack_from("<I", brut, pos + 20)[0]] == ouverts.pop()
        else:
            raise AssertionError(f"bloc inattendu {genre:#x}")
        pos += taille
    assert pos == len(brut) and not ouverts and espaces == 0
    return dict(elements), identifiants


def lire_ressources(brut):
    genre, entete, taille, paquets = struct.unpack_from("<HHII", brut, 0)
    assert (genre, entete, taille, paquets) == (0x0002, 12, len(brut), 1)
    valeurs, pos = lire_reserve(brut, 12)
    genre, entete, taille, identifiant = struct.unpack_from("<HHII", brut, pos)
    assert (genre, entete, identifiant) == (0x0200, 288, 0x7F) and pos + taille == len(brut)
    nom = brut[pos + 12 : pos + 268].decode("utf-16-le").rstrip("\0")
    types, cles_pos = lire_reserve(brut, pos + struct.unpack_from("<I", brut, pos + 268)[0])
    cles, suite = lire_reserve(brut, pos + struct.unpack_from("<I", brut, pos + 276)[0])
    assert types == ["string"] and cles_pos == pos + struct.unpack_from("<I", brut, pos + 276)[0]
    configurations = {}
    while suite < len(brut):
        genre, entete, taille = struct.unpack_from("<HHI", brut, suite)
        if genre == 0x0202:
            assert struct.unpack_from("<I", brut, suite + 12)[0] == len(cles)
        else:
            assert genre == 0x0201 and brut[suite + 8] == 1
            nombre, debut = struct.unpack_from("<II", brut, suite + 12)
            langue = brut[suite + 28 : suite + 30].rstrip(b"\0").decode()
            table = {}
            for rang in range(nombre):
                ecart = struct.unpack_from("<I", brut, suite + entete + 4 * rang)[0]
                if ecart == 0xFFFFFFFF:
                    continue
                _, _, cle, _, _, type_valeur, donnee = struct.unpack_from(
                    "<HHIHBBI", brut, suite + debut + ecart
                )
                assert type_valeur == 0x03 and cle == rang
                table[cles[cle]] = valeurs[donnee]
            configurations[langue] = table
        suite += taille
    return nom, cles, configurations


def decouper(brut):
    champs, pos = [], 0
    while pos < len(brut):
        if pos + 4 > len(brut):
            raise EchecSignature("préfixe de longueur tronqué")
        taille = struct.unpack_from("<I", brut, pos)[0]
        if pos + 4 + taille > len(brut):
            raise EchecSignature("champ qui sort de son conteneur")
        champs.append(brut[pos + 4 : pos + 4 + taille])
        pos += 4 + taille
    return champs


def sections_apk(apk):
    fin = apk.rfind(b"PK\x05\x06")
    if fin < 0 or fin + 22 != len(apk):
        raise EchecSignature("fin de répertoire central introuvable")
    repertoire = struct.unpack_from("<I", apk, fin + 16)[0]
    if repertoire < 32 or apk[repertoire - 16 : repertoire] != b"APK Sig Block 42":
        raise EchecSignature("pas de bloc de signature devant le répertoire central")
    taille = struct.unpack_from("<Q", apk, repertoire - 24)[0]
    debut = repertoire - taille - 8
    if debut < 0 or struct.unpack_from("<Q", apk, debut)[0] != taille:
        raise EchecSignature("tailles du bloc de signature incohérentes")
    return debut, repertoire, fin


def valeur_v2(apk, debut, repertoire):
    pos = debut + 8
    while pos < repertoire - 24:
        taille, identifiant = struct.unpack_from("<QI", apk, pos)
        if identifiant == ID_V2:
            return apk[pos + 12 : pos + 8 + taille]
        pos += 8 + taille
    raise EchecSignature("bloc v2 absent")


def condensat_attendu(apk, debut, repertoire, fin):
    queue = bytearray(apk[fin:])
    struct.pack_into("<I", queue, 16, debut)
    morceaux = []
    for section in (apk[:debut], apk[repertoire:fin], bytes(queue)):
        for rang in range(0, len(section), 1 << 20):
            bloc = section[rang : rang + (1 << 20)]
            morceaux.append(hashlib.sha256(b"\xa5" + struct.pack("<I", len(bloc)) + bloc).digest())
    tete = b"\x5a" + struct.pack("<I", len(morceaux))
    return hashlib.sha256(tete + b"".join(morceaux)).digest()


def verifier_v2(apk, nom):
    debut, repertoire, fin = sections_apk(apk)
    (signataires,) = decouper(valeur_v2(apk, debut, repertoire))
    (signataire,) = decouper(signataires)
    signees, signatures, publique = decouper(signataire)
    condensats, certificats, attributs = decouper(signees)
    (condensat,) = decouper(condensats)
    (signature,) = decouper(signatures)
    (certificat,) = decouper(certificats)
    if struct.unpack_from("<I", condensat)[0] != 0x0103 or signature[:4] != condensat[:4]:
        raise EchecSignature("algorithme inattendu")
    if decouper(condensat[4:]) != [condensat_attendu(apk, debut, repertoire, fin)]:
        raise EchecSignature("condensat du contenu différent")
    dossier = SORTIE / nom
    dossier.mkdir(parents=True, exist_ok=True)
    (dossier / "cert.der").write_bytes(certificat)
    (dossier / "signees.bin").write_bytes(signees)
    (dossier / "signature.bin").write_bytes(decouper(signature[4:])[0])
    pem = subprocess.run(
        [
            "openssl",
            "x509",
            "-inform",
            "DER",
            "-in",
            str(dossier / "cert.der"),
            "-pubkey",
            "-noout",
        ],
        capture_output=True,
        check=True,
    ).stdout
    (dossier / "publique.pem").write_bytes(pem)
    der = subprocess.run(
        ["openssl", "pkey", "-pubin", "-outform", "DER"], input=pem, capture_output=True, check=True
    ).stdout
    if der != publique:
        raise EchecSignature("clé publique différente de celle du certificat")
    commande = ["openssl", "dgst", "-sha256", "-verify", str(dossier / "publique.pem")]
    commande += ["-signature", str(dossier / "signature.bin"), str(dossier / "signees.bin")]
    if subprocess.run(commande, capture_output=True).returncode != 0:
        raise EchecSignature("signature RSA refusée par openssl")
    return attributs


def test_zip_archive_relue_par_zipfile_et_unzip(apk, dex):
    chemin = SORTIE / "bonjour.apk"
    chemin.write_bytes(apk)
    with zipfile.ZipFile(io.BytesIO(apk)) as archive:
        assert archive.namelist() == ["AndroidManifest.xml", "classes.dex", "resources.arsc"]
        assert archive.testzip() is None
        assert archive.read("classes.dex") == dex
        infos = {i.filename: i for i in archive.infolist()}
    assert infos["classes.dex"].compress_type == zipfile.ZIP_DEFLATED
    stockee = infos["resources.arsc"]
    assert stockee.compress_type == zipfile.ZIP_STORED
    nom, extra = struct.unpack_from("<HH", apk, stockee.header_offset + 26)
    assert (stockee.header_offset + 30 + nom + extra) % 4 == 0
    resultat = subprocess.run(["unzip", "-t", str(chemin)], capture_output=True, text=True)
    assert resultat.returncode == 0, resultat.stdout + resultat.stderr


def test_manifeste_paquet_versions_activite_et_filtre(description):
    elements, identifiants = lire_manifeste(mkapk.manifeste_binaire(description))
    assert identifiants == sorted(identifiants)
    racine = elements["manifest"]
    assert racine["package"] == (0, 0x03, "com.velum.bonjour")
    assert racine["versionCode"] == (0x0101021B, 0x10, 1)
    assert racine["versionName"] == (0x0101021C, 0x03, "1.0")
    sdk = elements["manifest/uses-sdk"]
    assert sdk["minSdkVersion"] == (0x0101020C, 0x10, 21)
    assert sdk["targetSdkVersion"] == (0x01010270, 0x10, 34)
    assert elements["manifest/application"]["label"] == (0x01010001, 0x01, 0x7F010001)
    activite = elements["manifest/application/activity"]
    assert activite["name"] == (0x01010003, 0x03, "com.velum.bonjour.Principale")
    assert activite["exported"] == (0x01010010, 0x12, 0xFFFFFFFF)
    filtre = "manifest/application/activity/intent-filter"
    assert elements[filtre + "/action"]["name"][2] == "android.intent.action.MAIN"
    assert elements[filtre + "/category"]["name"][2] == "android.intent.category.LAUNCHER"


def test_manifeste_libelle_en_clair_et_permissions(description):
    autre = mkapk.Description(**{**vars(description), "libelle": "Essai", "permissions": ["a.B"]})
    elements, _ = lire_manifeste(mkapk.manifeste_binaire(autre))
    assert elements["manifest/application"]["label"] == (0x01010001, 0x03, "Essai")
    assert elements["manifest/uses-permission"]["name"][2] == "a.B"


def test_ressources_chaines_par_defaut_et_fr(description):
    nom, cles, configurations = lire_ressources(mkapk.ressources_binaires(description))
    assert nom == "com.velum.bonjour" and cles == ["accueil", "nom_appli"]
    assert configurations[""] == {"accueil": "Hello from an APK", "nom_appli": "Bonjour APK"}
    assert configurations["fr"]["accueil"] == "Bonjour depuis un APK"


def test_signature_v2_condensats_et_rsa_reverifies(apk):
    assert verifier_v2(apk, "verif-valide") == b""
    with zipfile.ZipFile(io.BytesIO(apk)) as archive:
        assert archive.testzip() is None


def test_signature_refusee_si_contenu_altere(apk, dex, description):
    with pytest.raises(EchecSignature, match="condensat"):
        verifier_v2(mkapk.fabriquer(description, dex, CLES, 40), "verif-contenu")
    debut, repertoire, fin = sections_apk(apk)
    with pytest.raises(EchecSignature, match="condensat"):
        verifier_v2(mkapk.alterer(apk, repertoire + 20), "verif-repertoire")
    with pytest.raises(EchecSignature, match="condensat"):
        verifier_v2(mkapk.alterer(apk, fin + 10), "verif-fin")


def test_signature_refusee_si_bloc_altere(apk):
    debut, repertoire, _ = sections_apk(apk)
    signature = apk.index(struct.pack("<II", 0x0103, 256), debut + 200)
    with pytest.raises(EchecSignature, match="openssl"):
        verifier_v2(mkapk.alterer(apk, signature + 8 + 100), "verif-signature")
    with pytest.raises(EchecSignature):
        verifier_v2(mkapk.alterer(apk, repertoire - 1), "verif-magie")


def test_signature_absente_refusee(dex, description):
    nu = mkapk.fabriquer(description, dex)
    with pytest.raises(EchecSignature, match="pas de bloc"):
        verifier_v2(nu, "verif-nu")
    with zipfile.ZipFile(io.BytesIO(nu)) as archive:
        assert archive.testzip() is None


def test_ligne_de_commande_signe_altere_et_sans_signature(dex, capsys):
    SORTIE.mkdir(parents=True, exist_ok=True)
    (SORTIE / "classes.dex").write_bytes(dex)
    base = [str(BONJOUR / "bonjour.toml"), "--dex", str(SORTIE / "classes.dex")]
    signe, altere, nu = (SORTIE / n for n in ("cli.apk", "cli-altere.apk", "cli-nu.apk"))
    assert mkapk.main([*base, "--sortie", str(signe), "--cles", str(CLES)]) == 0
    verifier_v2(signe.read_bytes(), "verif-cli")
    arguments = [*base, "--sortie", str(altere), "--cles", str(CLES), "--alterer", "100"]
    assert mkapk.main(arguments) == 0
    with pytest.raises(EchecSignature):
        verifier_v2(altere.read_bytes(), "verif-cli-altere")
    assert mkapk.main([*base, "--sortie", str(nu), "--sans-signature"]) == 0
    with pytest.raises(EchecSignature):
        verifier_v2(nu.read_bytes(), "verif-cli-nu")
    assert mkapk.main([*base, "--sortie", str(nu)]) == 1
    assert "--cles" in capsys.readouterr().err


def test_cles_refusees_dans_le_depot_hors_de_build():
    with pytest.raises(mkapk.Erreur, match="build"):
        mkapk.dossier_cles(RACINE / "user" / "apk" / "cles")
    assert not (RACINE / "user" / "apk" / "cles").exists()


@pytest.mark.parametrize(
    ("remplacement", "fragment"),
    [
        (('paquet = "com.velum.bonjour"', 'paquet = "sans point"'), "paquet"),
        (("version_code = 1", "version_code = 0"), "version_code"),
        (("version_code = 1\n", ""), "absente"),
        (("sdk_cible = 34", "sdk_cible = 3"), "sdk_cible"),
        (("@string/nom_appli", "@string/inconnue"), "inconnue"),
        (("[chaines.fr]", "[chaines.de]"), "defaut et fr"),
        (("permissions = []", "permissions = [3]"), "permissions"),
    ],
)
def test_description_invalide_refusee(remplacement, fragment):
    texte = (BONJOUR / "bonjour.toml").read_text(encoding="utf-8")
    assert remplacement[0] in texte
    with pytest.raises(mkapk.Erreur, match=fragment):
        mkapk.lire_description(texte.replace(*remplacement))
