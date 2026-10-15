import re

NAME = "a01 : fault=pf donne une panique avec CR2 et RIP exacts"
CMDLINE = "selftest exit fault=pf"


def run(vm):
    vm.attendre(r"PANIC: ", delai=60)
    texte = vm.serie()
    garde = re.search(r"fault: page de garde (0x[0-9a-f]+)", texte).group(1)
    attendu = re.search(r"fault: rip attendu (0x[0-9a-f]+)", texte).group(1)
    assert f"cr2={garde} " in texte, texte[-2000:]
    assert f"rip={attendu} " in texte, texte[-2000:]
    panique = re.search(r"PANIC: (.*)", texte).group(1)
    assert panique.startswith("#PF") or garde in panique, panique
