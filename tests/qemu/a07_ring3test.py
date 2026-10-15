NAME = "ring3test : appels hostiles, fautes user, enfants, fils"
CMDLINE = "init=/system/bin/ring3test"
FIRMWARES = ("bios", "uefi")


def run(vm):
    vm.attendre(r"RING3TEST début", delai=60)
    vm.attendre(r"RING3TEST (PASS|FAIL \d+)", delai=120)
    texte = vm.serie()
    assert "RING3TEST FAIL" not in texte, texte[-3000:]
    assert "processus ring3test" in texte and "#PF" in texte, texte[-3000:]
    assert "#DE" in texte, texte[-3000:]
    assert "[ok] pile : mappage sur la garde refusé" in texte, texte[-3000:]
    assert "[ok] pile : débordement tué par faute de page" in texte, texte[-3000:]
    assert "[ok] nprocs revenu après la récolte" in texte, texte[-3000:]
    assert "PANIC:" not in texte, texte[-3000:]
