import re

NAME = "a09 : generateur aleatoire (graine materielle, autotest, appels getrandom et sysinfo)"
CMDLINE = "selftest exit"


def run(vm):
    vm.attendre(r"random: graine rdseed=\w+ rdrand=\w+ gigue=oui horloge=\w+", delai=60)
    vm.attendre(r"boot: random ok", delai=60)
    vm.attendre(r"\[TEST\] random \.\.\. OK", delai=120)
    texte = vm.serie()
    assert not re.search(r"random: (vecteur|controle)", texte), "autotest random en echec"
