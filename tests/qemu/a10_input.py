NAME = "clavier et souris PS/2 : sendkey et mouse_move"
CMDLINE = "selftest inputlog"
FIRMWARES = ("bios", "uefi")


def run(vm):
    vm.attendre(r"input: clavier PS/2 prêt", delai=60)
    vm.attendre(r"input: souris PS/2 prête \(id 3\)", delai=10)
    vm.attendre(r"\[TEST\] input \.\.\. OK", delai=30)
    vm.touche("a")
    vm.attendre(r"inp key_down code=0x51 sc=0x1c ", delai=10)
    vm.attendre(r"inp char code=0x71 ", delai=10)
    vm.touche("ret")
    vm.attendre(r"inp char code=0xd ", delai=10)
    vm.touche("shift-a")
    vm.attendre(r"inp char code=0x51 ", delai=10)
    vm.touche("up")
    vm.attendre(r"inp key_down code=0x26 sc=0xe075 mods=0x[a-f0-9]*", delai=10)
    vm.souris_deplacer(10, 5)
    vm.attendre(r"inp mouse_move code=0x0 sc=0x0 mods=0x[0-9a-f]+ x=10 y=5", delai=10)
    vm.souris_bouton(1)
    vm.attendre(r"inp mouse_down code=0x0 ", delai=10)
    vm.souris_bouton(0)
    vm.attendre(r"inp mouse_up code=0x0 ", delai=10)
