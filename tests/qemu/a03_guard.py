import re
import time

NAME = "mémoire virtuelle : page de garde d'une pile noyau (profil debug)"
CMDLINE = "selftest exit fault=guard"
MOTIF = r"PANIC: vmm: page de garde de pile noyau touchee a (0x[0-9a-f]+)"


def run(vm):
    vm.attendre(r"vmm selftest: ecriture sous la pile", delai=120)
    fin = time.monotonic() + 30
    while vm.processus.poll() is None and time.monotonic() < fin:
        time.sleep(0.05)
    texte = vm.serie()
    trouve = re.search(MOTIF, texte)
    assert trouve, texte[-2000:]
    adresse = int(trouve.group(1), 16)
    assert 0xFFFFE00000000000 <= adresse < 0xFFFFE10000000000, hex(adresse)
    assert "page de garde non detectee" not in texte
