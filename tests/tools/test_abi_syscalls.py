import re
from collections import Counter
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
ENTETE = RACINE / "include/velum/abi/abi_syscall.h"
FIGE = RACINE / "tests/abi/syscalls.golden"
DEFINITION = re.compile(r"^# define (SYS_[A-Z0-9_]+) (0x[0-9a-f]+)$", re.MULTILINE)
VERSION = re.compile(r"^# define VELUM_ABI_VERSION (\d+)$", re.MULTILINE)
LIMITE = re.compile(r"^# define SYS_MAX (0x[0-9a-f]+)$", re.MULTILINE)
STATUTS = {"actif", "retire"}


def lire_entete(texte):
    return [(nom, int(numero, 16)) for nom, numero in DEFINITION.findall(texte) if nom != "SYS_MAX"]


def lire_fige(texte):
    appels = []
    trous = []
    autres = {"version": None, "max": None}
    for ligne in texte.splitlines():
        champs = ligne.split()
        if not champs or ligne.startswith("#"):
            continue
        if champs[0] in autres:
            autres[champs[0]] = int(champs[1], 0)
        elif champs[0] == "trou":
            trous.append((int(champs[1], 16), int(champs[2], 16)))
        elif champs[0] != "taille":
            numero, nom, statut, arguments = champs
            appels.append((int(numero, 16), nom, statut, int(arguments)))
    return appels, trous, autres


def doublons(valeurs):
    return [valeur for valeur, nombre in Counter(valeurs).items() if nombre > 1]


def controle_entete(entete):
    fautes = []
    noms = [nom for nom, _ in entete]
    numeros = [numero for _, numero in entete]
    fautes += [f"nom en double dans l'en-tete : {n}" for n in doublons(noms)]
    fautes += [f"numero attribue deux fois : {n:#x}" for n in doublons(numeros)]
    return fautes


def controle_fige_seul(appels, trous, autres, limite):
    fautes = []
    fautes += [
        f"nom en double dans le golden : {n}" for n in doublons([nom for _, nom, _, _ in appels])
    ]
    fautes += [
        f"numero attribue deux fois dans le golden : {n:#x}"
        for n in doublons([numero for numero, _, _, _ in appels])
    ]
    for numero, nom, statut, arguments in appels:
        if statut not in STATUTS:
            fautes.append(f"statut inconnu : {nom}")
        if not 0 <= arguments <= 6:
            fautes.append(f"nombre d'arguments invalide : {nom}")
        if numero >= limite:
            fautes.append(f"numero au-dela de SYS_MAX : {nom}")
    attribues = {numero for numero, _, _, _ in appels}
    en_trou = set()
    for premier, dernier in trous:
        plage = set(range(premier, dernier + 1))
        if plage & en_trou:
            fautes.append(f"trou {premier:#x}-{dernier:#x} declare deux fois")
        if plage & attribues:
            fautes.append(f"numero attribue et declare en trou : {premier:#x}")
        en_trou |= plage
    inconnus = set(range(limite)) - attribues - en_trou
    fautes += [f"numero ni attribue ni declare en trou : {n:#x}" for n in sorted(inconnus)]
    return fautes, en_trou


def controle(texte_entete, texte_fige):
    entete = lire_entete(texte_entete)
    appels, trous, autres = lire_fige(texte_fige)
    limite = int(LIMITE.search(texte_entete).group(1), 16)
    fautes = controle_entete(entete)
    fautes_fige, en_trou = controle_fige_seul(appels, trous, autres, limite)
    fautes += fautes_fige
    version = int(VERSION.search(texte_entete).group(1))
    if autres["version"] != version:
        fautes.append(f"version ABI changee : {version}")
    if autres["max"] != limite:
        fautes.append(f"SYS_MAX change : {limite:#x}")
    par_nom = dict(entete)
    par_numero = {numero: nom for nom, numero in entete}
    figes_noms = {nom for _, nom, _, _ in appels}
    figes_numeros = {numero for numero, _, _, _ in appels}
    for numero, nom, statut, _ in appels:
        if statut == "actif":
            if nom not in par_nom:
                fautes.append(f"appel disparu sans retrait : {nom}")
            elif par_nom[nom] != numero:
                fautes.append(f"numero change : {nom}")
            if numero in par_numero and par_numero[numero] != nom:
                fautes.append(f"nom change ou numero reutilise : {nom}")
        elif statut == "retire":
            if nom in par_nom:
                fautes.append(f"retire mais encore defini : {nom}")
            if numero in par_numero:
                fautes.append(f"numero retire reattribue : {numero:#x}")
    for nom, numero in entete:
        if nom not in figes_noms and numero not in figes_numeros:
            fautes.append(f"appel absent du golden : {nom}")
        if numero in en_trou:
            fautes.append(f"numero dans un trou declare : {nom}")
    return fautes


@pytest.fixture(scope="module")
def entete():
    return ENTETE.read_text()


@pytest.fixture(scope="module")
def fige():
    return FIGE.read_text()


def test_table_courante_conforme_au_golden(entete, fige):
    assert controle(entete, fige) == []


def test_entete_assez_garni(entete):
    assert len(lire_entete(entete)) > 40


def test_numeros_uniques_dans_l_entete(entete):
    assert controle_entete(lire_entete(entete)) == []


def test_numero_jamais_reattribue(entete, fige):
    appels, _, _ = lire_fige(fige)
    entete_lue = dict(lire_entete(entete))
    for numero, nom, statut, _ in appels:
        if statut == "actif":
            assert entete_lue.get(nom) == numero, f"{nom} : numero change ou disparu"
        else:
            assert nom not in entete_lue, f"{nom} retire mais encore defini"
            assert numero not in entete_lue.values(), f"{numero:#x} reattribue"


def test_nouvel_appel_ajoute_a_la_liste_figee(entete, fige):
    appels, _, _ = lire_fige(fige)
    figes = {nom for _, nom, _, _ in appels}
    for nom, numero in lire_entete(entete):
        assert nom in figes, f"{nom} ({numero:#x}) absent de tests/abi/syscalls.golden"


def test_chaque_numero_attribue_ou_declare_en_trou(entete, fige):
    appels, trous, autres = lire_fige(fige)
    limite = int(LIMITE.search(entete).group(1), 16)
    fautes, _ = controle_fige_seul(appels, trous, autres, limite)
    assert fautes == []


MUTATIONS = [
    (
        "reutilisation d'un numero actif par un nouvel appel",
        "entete",
        "# define SYS_MAX 0xff",
        "# define SYS_NOUVEAU 0x07\n# define SYS_MAX 0xff",
        "numero attribue deux fois",
    ),
    (
        "reutilisation d'un numero actif, ancien nom remplace",
        "entete",
        "SYS_GETTID 0x07",
        "SYS_AUTRE 0x07",
        "nom change ou numero reutilise",
    ),
    (
        "reutilisation d'un numero retire",
        "les_deux",
        ("0x50 SYS_YIELD actif 0", "0x50 SYS_YIELD retire 0"),
        ("SYS_YIELD 0x50", "SYS_AUTRE 0x50"),
        "numero retire reattribue",
    ),
    (
        "renumerotation vers un numero libre",
        "entete",
        "SYS_GETTID 0x07",
        "SYS_GETTID 0x0b",
        "numero change",
    ),
    (
        "renumerotation vers un numero deja pris",
        "entete",
        "SYS_GETTID 0x07",
        "SYS_GETTID 0x06",
        "numero attribue deux fois",
    ),
    ("echange de deux numeros", "entete", "SYS_READ 0x31", "SYS_READ 0x32", "numero change"),
    (
        "renumerotation vers un trou declare",
        "entete",
        "SYS_YIELD 0x50",
        "SYS_YIELD 0x55",
        "numero dans un trou declare",
    ),
    (
        "suppression sans retrait",
        "entete",
        "# define SYS_LOG 0x0a\n",
        "",
        "appel disparu sans retrait",
    ),
    (
        "retrait declare mais appel encore defini",
        "fige",
        "0x50 SYS_YIELD actif",
        "0x50 SYS_YIELD retire",
        "retire mais encore defini",
    ),
    ("ligne du golden supprimee", "fige", "0x31 SYS_READ actif 3\n", "", "appel absent du golden"),
    (
        "nom en double dans l'en-tete",
        "entete",
        "# define SYS_MAX 0xff",
        "# define SYS_READ 0x3e\n# define SYS_MAX 0xff",
        "nom en double",
    ),
    (
        "appel sur la limite SYS_MAX",
        "entete",
        "# define SYS_MAX 0xff",
        "# define SYS_NOUVEAU 0xff\n# define SYS_MAX 0xff",
        "absent du golden",
    ),
    (
        "nouvel appel dans un trou",
        "entete",
        "# define SYS_MAX 0xff",
        "# define SYS_NOUVEAU 0x20\n# define SYS_MAX 0xff",
        "numero dans un trou declare",
    ),
    ("trou supprime", "fige", "trou 0x18 0x18\n", "", "ni attribue ni declare"),
    (
        "trou recouvrant un appel",
        "fige",
        "trou 0x18 0x18\n",
        "trou 0x18 0x18\ntrou 0x10 0x10\n",
        "attribue et declare en trou",
    ),
    ("statut inconnu", "fige", "0x50 SYS_YIELD actif", "0x50 SYS_YIELD suspendu", "statut inconnu"),
    (
        "nom en double dans le golden",
        "fige",
        "0x51 SYS_SLEEP",
        "0x51 SYS_YIELD",
        "nom en double dans le golden",
    ),
    (
        "version ABI changee",
        "entete",
        "VELUM_ABI_VERSION 1",
        "VELUM_ABI_VERSION 2",
        "version ABI changee",
    ),
    (
        "limite SYS_MAX changee",
        "entete",
        "# define SYS_MAX 0xff",
        "# define SYS_MAX 0x7f",
        "SYS_MAX change",
    ),
]


def appliquer(texte, ancien, nouveau):
    assert texte.count(ancien) == 1, f"mutation inapplicable : {ancien!r}"
    return texte.replace(ancien, nouveau)


@pytest.mark.parametrize(
    "nom,cible,ancien,nouveau,attendu", MUTATIONS, ids=[m[0] for m in MUTATIONS]
)
def test_mutation_detectee(entete, fige, nom, cible, ancien, nouveau, attendu):
    texte_entete, texte_fige = entete, fige
    if cible == "entete":
        texte_entete = appliquer(entete, ancien, nouveau)
    elif cible == "fige":
        texte_fige = appliquer(fige, ancien, nouveau)
    else:
        texte_fige = appliquer(fige, ancien[0], ancien[1])
        texte_entete = appliquer(entete, nouveau[0], nouveau[1])
    fautes = controle(texte_entete, texte_fige)
    assert any(attendu in faute for faute in fautes), f"{nom} : {fautes}"


def test_retrait_legitime_accepte(entete, fige):
    texte_fige = appliquer(fige, "0x50 SYS_YIELD actif 0", "0x50 SYS_YIELD retire 0")
    texte_entete = appliquer(entete, "# define SYS_YIELD 0x50\n", "")
    assert controle(texte_entete, texte_fige) == []
