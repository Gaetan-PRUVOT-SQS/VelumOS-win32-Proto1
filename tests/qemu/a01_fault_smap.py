import re

NAME = "a01 : fault=smap, lecture d'une page utilisateur sans STAC donne #PF (P=1, I=0)"
CMDLINE = "selftest exit fault=smap"
QEMU_EXTRA = ("-cpu", "max")


def run(vm):
    vm.attendre(r"PANIC: |fault=smap (indisponible|n'a pas)", delai=60)
    texte = vm.serie()
    assert "indisponible" not in texte, f"SMAP absent du CPU\n{texte[-2000:]}"
    assert "n'a pas arrêté le noyau" not in texte, f"SMAP n'agit pas\n{texte[-2000:]}"
    page = re.search(r"fault: smap page utilisateur (0x[0-9a-f]+)", texte).group(1)
    site = re.search(r"fault: rip attendu (0x[0-9a-f]+)", texte).group(1)
    assert re.search(r"PANIC: vmm: faute de page noyau a %s \[PRS--\]" % page, texte), texte[-2000:]
    assert f"cr2={page} " in texte, texte[-2000:]
    assert f"rip={site} " in texte, texte[-2000:]
    assert re.search(r"vec=14 err=0x1 rip=%s " % site, texte), texte[-2000:]
