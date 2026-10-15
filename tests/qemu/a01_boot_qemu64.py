NAME = "a01 : processeur minimal (qemu64), repli FXSAVE et protections absentes"
CMDLINE = "selftest exit"
QEMU_EXTRA = ("-cpu", "qemu64")


def run(vm):
    ligne = vm.attendre(r"cpu: .*, fpu (\d+) o, ([a-z0-9 ]*)\n", delai=60)
    capacites = ligne.group(2).split()
    assert ligne.group(1) == "512", ligne.group(0)
    assert "xsave" not in capacites and "smep" not in capacites, ligne.group(0)
    vm.attendre(r"\[TEST\] cpu \.\.\. OK", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
