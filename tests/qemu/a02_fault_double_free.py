import re
import time

from vtest import VMError

NAME = "a02 : libération invalide (double) : panique du noyau"
CMDLINE = "selftest exit pmm_fault=double"
MOTIF = r"PANIC: assertion: pmm: double libération"


def attendre_panique(vm, delai=60):
    fin = time.monotonic() + delai
    while time.monotonic() < fin and not re.search(MOTIF, vm.serie()):
        if vm.processus.poll() is not None:
            break
        time.sleep(0.05)
    texte = vm.serie()
    if not re.search(MOTIF, texte):
        raise VMError(f"panique attendue absente : {MOTIF!r}\n{texte[-1500:]}")
    return texte


def run(vm):
    texte = attendre_panique(vm)
    assert re.search(r"phys=0x[0-9a-f]+ n=1 détenteur=-?\d+ appelant=0x[0-9a-f]+", texte)
    assert "SELFTESTS PASS" not in texte, "l'autotest a continué après la panique"
