NAME = "tas noyau : autotest avec 64 Mio (BIOS)"
CMDLINE = "selftest exit"
MEMOIRE = "64M"
FIRMWARES = ("bios",)


def run(vm):
    vm.attendre(r"boot: heap ok", delai=60)
    vm.attendre(r"heap: au repos \d+ objets", delai=120)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
