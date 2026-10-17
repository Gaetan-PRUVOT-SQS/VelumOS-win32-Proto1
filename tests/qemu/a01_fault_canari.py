import re

NAME = "a01 : fault=canari, canari de pile écrasé donne la panique stack smashing"
CMDLINE = "selftest exit fault=canari"
QEMU_EXTRA = ("-cpu", "max")


def run(vm):
    vm.attendre(r"PANIC: ", delai=60)
    texte = vm.serie()
    debut = re.search(r"fault: canari fonction (0x[0-9a-f]+) fenetre (\d+)", texte)
    assert debut, texte[-2000:]
    panique = re.search(r"PANIC: stack smashing detected \(from (0x[0-9a-f]+)\)", texte)
    assert panique, texte[-2000:]
    origine = int(panique.group(1), 16)
    base = int(debut.group(1), 16)
    assert base <= origine < base + int(debut.group(2)), (hex(origine), hex(base))
    assert "n'a pas arrêté le noyau" not in texte, texte[-2000:]
