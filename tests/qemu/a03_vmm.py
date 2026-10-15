NAME = "mémoire virtuelle : tables propres, autotest, audit W^X"
CMDLINE = "selftest exit"


def run(vm):
    vm.attendre(r"vmm: tables \d+ Kio, hhdm \d+ plages", delai=60)
    vm.attendre(r"boot: vmm ok", delai=60)
    vm.attendre(r"\[TEST\] vmm \.\.\. OK", delai=120)
