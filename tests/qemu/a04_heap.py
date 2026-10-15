import re

NAME = "tas noyau : démarrage et autotest"
CMDLINE = "selftest exit"
MEMOIRE = "256M"


def run(vm):
    vm.attendre(r"heap: 14 classes", delai=60)
    vm.attendre(r"boot: heap ok", delai=5)
    vm.attendre(r"heap: au repos \d+ objets", delai=120)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
    texte = vm.serie()
    assert "en echec" not in texte, "un essai du tas a échoué"
    assert not re.search(r"heap: .*[1-9][0-9]* anomalie", texte), (
        "heap_check a signalé une anomalie"
    )
