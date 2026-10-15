import re

NAME = "endurance : 320 fenêtres ouvertes puis fermées avec 64 Mo, mémoire libre stable"
CMDLINE = "quiet init=/system/bin/wstress"
MEMOIRE = "64M"
FIRMWARES = ("bios",)

CYCLES_MINI = 300
DERIVE_MAXI_KO = 512
BILAN = re.compile(r"WSTRESS PASS cycles (\d+) avant (\d+) apres (\d+)")


def run(vm):
    vm.attendre(r"WSTRESS start", delai=60)
    vm.attendre(r"WSTRESS (PASS|FAIL) .*", delai=240)
    texte = vm.serie()
    assert "PANIC:" not in texte, texte[-2000:]
    assert "WSTRESS FAIL" not in texte, texte[-2000:]
    bilan = BILAN.search(texte)
    assert bilan, texte[-2000:]
    cycles, avant, apres = (int(valeur) for valeur in bilan.groups())
    assert cycles >= CYCLES_MINI, f"{cycles} cycles joués, {CYCLES_MINI} attendus"
    derive = avant - apres
    assert derive <= DERIVE_MAXI_KO, (
        f"mémoire libre : {avant} Ko avant, {apres} Ko après, dérive {derive} Ko "
        f"pour {cycles} cycles (borne {DERIVE_MAXI_KO} Ko)"
    )
