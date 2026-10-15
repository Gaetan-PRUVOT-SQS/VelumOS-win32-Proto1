NAME = "init absent : le noyau le dit et continue"
CMDLINE = "init=/system/bin/absent"
FIRMWARES = ("bios",)


def run(vm):
    vm.attendre(r"init: /system/bin/absent absent, le noyau continue", delai=60)
    texte = vm.serie()
    assert "PANIC:" not in texte, texte[-2000:]
