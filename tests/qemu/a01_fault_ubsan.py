import re

NAME = "a01 : fault=ubsan, décalage hors borne piégé par UBSan donne #UD dans la fonction instrumentée"
CMDLINE = "selftest exit fault=ubsan"
QEMU_EXTRA = ("-cpu", "max")


def run(vm):
    vm.attendre(r"PANIC: ", delai=60)
    texte = vm.serie()
    debut = re.search(r"fault: ubsan fonction (0x[0-9a-f]+) fenetre (\d+)", texte)
    assert debut, texte[-2000:]
    assert re.search(r"PANIC: #UD ", texte), texte[-2000:]
    rip = re.search(r"rip=(0x[0-9a-f]+) ", texte)
    assert rip, texte[-2000:]
    base = int(debut.group(1), 16)
    assert base <= int(rip.group(1), 16) < base + int(debut.group(2)), (rip.group(1), hex(base))
    assert "n'a pas arrêté le noyau" not in texte, texte[-2000:]
