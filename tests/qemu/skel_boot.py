NAME = "démarrage du noyau squelette"
CMDLINE = "selftest exit"


def run(vm):
    vm.attendre(r"VelumOS boot ok", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
