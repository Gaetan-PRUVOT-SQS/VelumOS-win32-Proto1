import re
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
ENTETE = RACINE / "include/velum/abi/abi_syscall.h"
FIGE = RACINE / "tests/abi/syscalls.golden"
DEFINITION = re.compile(r"^# define (SYS_[A-Z0-9_]+) (0x[0-9a-f]+)$", re.MULTILINE)
STATUTS = {"actif", "retire"}


def lire_entete():
    appels = {}
    for nom, numero in DEFINITION.findall(ENTETE.read_text()):
        if nom != "SYS_MAX":
            appels[nom] = int(numero, 16)
    return appels


def lire_fige():
    appels = {}
    for ligne in FIGE.read_text().splitlines():
        if not ligne or ligne.startswith("#"):
            continue
        numero, nom, statut = ligne.split()
        assert statut in STATUTS, f"statut inconnu : {ligne}"
        assert nom not in appels, f"nom en double dans la liste figée : {nom}"
        appels[nom] = (int(numero, 16), statut)
    return appels


def test_numeros_uniques_dans_l_entete():
    appels = lire_entete()
    numeros = list(appels.values())
    assert len(appels) > 40
    assert len(set(numeros)) == len(numeros), "un numéro d'appel est attribué deux fois"


def test_numero_jamais_reattribue():
    entete = lire_entete()
    for nom, (numero, statut) in lire_fige().items():
        if statut == "actif":
            assert entete.get(nom) == numero, f"{nom} : numéro changé ou appel disparu"
        else:
            assert nom not in entete, f"{nom} retiré mais encore défini"
            assert numero not in entete.values(), f"numéro retiré {numero:#x} réattribué"


def test_nouvel_appel_ajoute_a_la_liste_figee():
    figes = lire_fige()
    for nom, numero in lire_entete().items():
        assert nom in figes, f"{nom} ({numero:#x}) absent de tests/abi/syscalls.golden"


def test_numeros_figes_sous_la_limite():
    limite = int(re.search(r"# define SYS_MAX (0x[0-9a-f]+)", ENTETE.read_text()).group(1), 16)
    assert all(numero < limite for numero, _ in lire_fige().values())
