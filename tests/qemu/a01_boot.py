NAME = "a01 : processeur au démarrage et autotest"
CMDLINE = "selftest exit"


def run(vm):
    ligne = vm.attendre(
        r"cpu: (\S+) \d+/\d+/\d+ .*pa \d+ va \d+, fpu (\d+) o, ([a-z0-9 ]*)", delai=60
    )
    assert ligne.group(1) in ("GenuineIntel", "AuthenticAMD"), ligne.group(0)
    assert int(ligne.group(2)) >= 512, ligne.group(0)
    assert "nx" in ligne.group(3).split(), ligne.group(0)
    vm.attendre(r"boot: cpu ok", delai=30)
    vm.attendre(r"\[TEST\] cpu \.\.\. OK", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
    assert "FAIL" not in vm.serie(), vm.serie()[-2000:]
