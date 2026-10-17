import re

NAME = "a01 : fault=smep, exécution d'une page utilisateur en anneau 0 donne #PF (I=1, P=1)"
CMDLINE = "selftest exit fault=smep"
QEMU_EXTRA = ("-cpu", "max")


def run(vm):
    vm.attendre(r"PANIC: |fault=smep (indisponible|n'a pas)", delai=60)
    texte = vm.serie()
    assert "indisponible" not in texte, f"SMEP absent du CPU\n{texte[-2000:]}"
    assert "n'a pas arrêté le noyau" not in texte, f"SMEP n'agit pas\n{texte[-2000:]}"
    page = re.search(r"fault: smep page utilisateur (0x[0-9a-f]+)", texte).group(1)
    assert re.search(r"PANIC: vmm: faute de page noyau a %s \[PRS-I\]" % page, texte), texte[-2000:]
    assert f"cr2={page} " in texte, texte[-2000:]
    assert f"rip={page} " in texte, texte[-2000:]
    assert re.search(r"vec=14 err=0x11 rip=%s " % page, texte), texte[-2000:]
