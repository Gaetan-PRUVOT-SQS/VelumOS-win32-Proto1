import re

NAME = "a02 : mémoire physique avec 8 Gio (trou PCI, RAM haute, autotests)"
CMDLINE = "selftest exit"
MEMOIRE = "8G"
GIO = 1024 * 1024
LIGNE_TOTAUX = r"pmm: total=(\d+) libre=(\d+) meta=(\d+) reserve=(\d+) Kio"
LIGNE_TAS = r"heap: fenetre (\d+) Mio, dalles de (\d+) Mio, grands blocs (\d+) Mio"


def metadonnees_attendues_kio(pages_gerees):
    mots = -(-pages_gerees // 64)
    octets = mots * 8 + pages_gerees
    return -(-octets // 4096) * 4


def mesurer(vm, memoire_gio, tas_dalle_mio):
    vm.attendre(LIGNE_TOTAUX, delai=60)
    vm.attendre(r"\[TEST\] pmm \.\.\. OK", delai=120)
    vm.attendre(r"\[TEST\] heap \.\.\. OK", delai=120)
    vm.attendre(r"SELFTESTS PASS \d+", delai=120)
    texte = vm.serie()
    total, libre, meta, reserve = map(int, re.search(LIGNE_TOTAUX, texte).groups())
    assert libre == total, f"libre {libre} != total {total} avant tout usage"
    assert total >= int(memoire_gio * 0.95 * GIO), f"total {total} Kio : RAM haute ignorée"
    assert total + reserve >= memoire_gio * GIO, "le span ne couvre pas toute la RAM"
    pages = (total + reserve) // 4
    assert meta == metadonnees_attendues_kio(pages), f"meta {meta} Kio pour {pages} pages"
    hautes = re.findall(r"pmm: plage 0x([0-9a-f]+)-0x[0-9a-f]+ libre", texte)
    assert any(int(debut, 16) >= 1 << 32 for debut in hautes), (
        "aucune plage libre au-dessus de 4 Gio"
    )
    tas = re.search(LIGNE_TAS, texte)
    assert tas, "ligne de disposition du tas absente"
    fenetre, dalle, grands = map(int, tas.groups())
    assert dalle == tas_dalle_mio, f"dalles de {dalle} Mio au lieu de {tas_dalle_mio} Mio"
    assert fenetre == 3 * dalle + grands, f"fenêtre {fenetre} Mio incohérente"


def run(vm):
    mesurer(vm, 8, 128)
