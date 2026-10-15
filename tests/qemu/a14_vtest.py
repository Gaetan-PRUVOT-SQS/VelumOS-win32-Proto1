NAME = "vtest : libvelum, libc, fils et verrous"
CMDLINE = "init=/system/bin/vtest"


def run(vm):
    vm.attendre(r"VTEST start", delai=60)
    vm.attendre(r"VTEST (PASS \d+|FAIL \d+/\d+)", delai=90)
    texte = vm.serie()
    assert "VTEST FAIL" not in texte, texte[-2000:]
    assert "VTEST PASS" in texte, texte[-2000:]
