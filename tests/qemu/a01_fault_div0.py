import re

NAME = "a01 : fault=div0 donne #DE avec les registres exacts"
CMDLINE = "selftest exit fault=div0"
TEMOINS = ("rbx=0x1111111111111111", "r12=0x1212121212121212", "r15=0x1515151515151515")


def verifier_panique(vm, nom, pile, rip_exact=True, extra=()):
    vm.attendre(rf"PANIC: {re.escape(nom)} ", delai=60)
    texte = vm.serie()
    motif = rf"exception {re.escape(nom)} \(.*\), vecteur \d+, anneau 0, pile (\S+)"
    entete = re.search(motif, texte)
    assert entete, texte[-2000:]
    assert entete.group(1) == pile, entete.group(0)
    for temoin in TEMOINS + tuple(extra):
        assert temoin in texte, f"{temoin} absent\n{texte[-2000:]}"
    if rip_exact:
        attendu = re.search(r"fault: rip attendu (0x[0-9a-f]+)", texte)
        assert attendu, texte[-2000:]
        assert f"rip={attendu.group(1)} " in texte, texte[-2000:]
    assert "trace : #0 " in texte, texte[-2000:]
    assert "faute imbriquée" not in texte, texte[-2000:]
    return texte


def run(vm):
    verifier_panique(vm, "#DE", "noyau", extra=("rax=0xa0a0 ", "rcx=0 ", "err=0 "))
