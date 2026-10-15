import re

NAME = "a02 : mémoire physique au démarrage"
CMDLINE = "selftest exit"
LIGNE_TOTAUX = r"pmm: total=(\d+) libre=(\d+) meta=(\d+) reserve=(\d+) Kio"


def metadonnees_attendues_kio(pages_gerees):
    mots = -(-pages_gerees // 64)
    octets = mots * 8 + pages_gerees
    return -(-octets // 4096) * 4


def run(vm):
    vm.attendre(LIGNE_TOTAUX, delai=60)
    vm.attendre(r"boot: pmm ok", delai=60)
    vm.attendre(r"\[TEST\] pmm \.\.\. OK", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
    texte = vm.serie()
    total, libre, meta, reserve = map(int, re.search(LIGNE_TOTAUX, texte).groups())
    assert libre == total, f"libre {libre} != total {total} avant tout usage"
    assert 128 * 1024 <= total <= 256 * 1024, f"total {total} Kio hors de la plage attendue"
    pages = (total + reserve) // 4
    assert meta == metadonnees_attendues_kio(pages), f"meta {meta} Kio pour {pages} pages"
    assert meta <= 96, f"métadonnées {meta} Kio : budget de 96 Kio dépassé pour 256 Mio"
    assert re.search(r"pmm: plage 0x[0-9a-f]+-0x[0-9a-f]+ libre", texte), "aucune plage journalisée"
