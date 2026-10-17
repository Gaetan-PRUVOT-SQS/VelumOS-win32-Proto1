import re

NAME = "a01 : fault=umip, sgdt en anneau 3 donne #GP (UMIP actif)"
CMDLINE = "selftest exit fault=umip"
QEMU_EXTRA = ("-cpu", "max")


def run(vm):
    vm.attendre(r"PANIC: |fault=umip (indisponible|n'a pas)", delai=60)
    texte = vm.serie()
    assert "indisponible" not in texte, f"UMIP absent du CPU\n{texte[-2000:]}"
    assert "n'a pas arrêté le noyau" not in texte, f"UMIP n'agit pas\n{texte[-2000:]}"
    site = re.search(r"fault: rip attendu (0x[0-9a-f]+)", texte).group(1)
    assert re.search(r"PANIC: faute en anneau 3 sans processus", texte), texte[-2000:]
    assert re.search(r"vec=13 err=0 rip=%s .* cs=0x23" % site, texte), texte[-2000:]
    assert f"rip={site} " in texte, texte[-2000:]
