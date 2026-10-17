import re

NAME = "kentry : AC utilisateur effacé à l'entrée des exceptions et IRQ, SMAP effectif"
CMDLINE = "init=/system/bin/ring3test entryaudit"
FIRMWARES = ("bios",)


def run(vm):
    vm.attendre(r"RING3TEST (PASS|FAIL \d+)", delai=180)
    texte = vm.serie()
    assert "RING3TEST PASS" in texte, texte[-3000:]
    assert "[ko]" not in texte, texte[-3000:]
    assert "audit entrée KO" not in texte, texte[-3000:]
    assert "PANIC:" not in texte, texte[-3000:]
    for nom in (
        "entrée : registres préservés par syscall",
        "entrée : AC utilisateur et appel système",
        "entrée : AC utilisateur et exception",
        "entrée : AC utilisateur et IRQ",
    ):
        assert f"[ok] {nom}" in texte, nom
    audits = re.findall(
        r"cpu: audit entrée vec (\d+), AC utilisateur 1, AC noyau 0, SMAP ([^\n]+)", texte
    )
    vecteurs = {int(v) for v, _ in audits}
    assert 6 in vecteurs, f"exception #UD non auditée : {audits}"
    assert any(v >= 32 for v in vecteurs), f"aucune IRQ auditée : {audits}"
    smap = re.search(r"cpu: .* smap ", texte) is not None
    verdicts = {s.strip() for _, s in audits}
    if smap:
        assert verdicts == {"effectif"}, f"SMAP non effectif à l'entrée : {audits}"
    else:
        assert verdicts == {"non testé"}, audits
