NAME = "objets, canaux et sections : autotest noyau"
CMDLINE = "selftest exit"
FIRMWARES = ("bios", "uefi")


def run(vm):
    vm.attendre(r"boot: object ok", delai=60)
    vm.attendre(r"\[TEST\] object \.\.\. OK", delai=120)
