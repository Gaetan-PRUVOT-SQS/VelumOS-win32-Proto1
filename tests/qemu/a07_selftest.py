import re

NAME = "appels système et processus : autotests noyau (1000 spawn/exit)"
CMDLINE = "selftest exit"
FIRMWARES = ("bios", "uefi")


def verdict(texte, nom):
    trouve = re.search(rf"\[TEST\] {nom} \.\.\. (.*?)(?=\[TEST\]|SELFTESTS)", texte, re.S)
    assert trouve, f"autotest {nom} absent\n{texte[-3000:]}"
    bloc = trouve.group(1)
    assert re.search(r"(^|\n|\] )OK\n", bloc) or bloc.rstrip().endswith("OK"), bloc
    assert "FAIL" not in bloc, bloc


def run(vm):
    vm.attendre(r"boot: syscall ok", delai=60)
    vm.attendre(r"boot: proc ok", delai=60)
    vm.attendre(r"SELFTESTS (PASS|FAIL)", delai=300)
    texte = vm.serie()
    verdict(texte, "syscall")
    verdict(texte, "proc")
    assert re.search(r"proc: 1000 lancements et fins en \d+ ms", texte), texte[-3000:]
    pages = re.search(r"proc: pages libres (\d+) avant, (\d+) apres", texte)
    assert pages and pages.group(1) == pages.group(2), texte[-3000:]
    assert "proc: garde de pile réservée" in texte, texte[-3000:]
    assert "PANIC:" not in texte, texte[-3000:]
