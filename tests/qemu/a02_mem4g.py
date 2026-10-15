import re

NAME = "a02 : mémoire physique avec 4 Gio (trou PCI, RAM au-dessus de 4 Gio)"
CMDLINE = "selftest exit"
MEMOIRE = "4G"
LIGNE_TOTAUX = r"pmm: total=(\d+) libre=(\d+) meta=(\d+) reserve=(\d+) Kio"


def run(vm):
    vm.attendre(LIGNE_TOTAUX, delai=60)
    vm.attendre(r"\[TEST\] pmm \.\.\. OK", delai=120)
    vm.attendre(r"SELFTESTS PASS \d+", delai=120)
    texte = vm.serie()
    total, libre, meta, reserve = map(int, re.search(LIGNE_TOTAUX, texte).groups())
    assert libre == total, f"libre {libre} != total {total} avant tout usage"
    assert total >= int(3.5 * 1024 * 1024), f"total {total} Kio : moins de 3,5 Gio vus"
    assert total + reserve >= 4 * 1024 * 1024, "le span ne dépasse pas 4 Gio : RAM haute ignorée"
    assert meta >= 1100, f"métadonnées {meta} Kio : trop petites pour un span de plus de 4 Gio"
    hautes = re.findall(r"pmm: plage 0x([0-9a-f]+)-0x[0-9a-f]+ libre", texte)
    assert any(int(debut, 16) >= 1 << 32 for debut in hautes), (
        "aucune plage libre au-dessus de 4 Gio"
    )
