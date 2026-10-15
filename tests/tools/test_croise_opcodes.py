import re
import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))
from dexasm_lib import opcodes  # noqa: E402

LIGNE = re.compile(r'^\{"([^"]+)", DF_(\w+), DK_(\w+),')
HORS_TRANCHE = {0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF}


def table_c():
    lignes = (RACINE / "lib" / "dexcode" / "dexcode_tab.c").read_text().splitlines()
    return [m.groups() for m in map(LIGNE.match, lignes) if m]


def test_la_table_c_a_256_entrees():
    assert len(table_c()) == 256


def test_meme_nom_et_meme_format_des_deux_cotes():
    table = table_c()
    assert len(opcodes.PAR_CODE) >= 200
    for code, (nom, fmt, _genre) in opcodes.PAR_CODE.items():
        nom_c, fmt_c, _genre_c = table[code]
        assert nom_c == nom, f"code {code:#04x} : {nom_c} en C, {nom} dans l'assembleur"
        assert fmt_c.lower() == fmt, f"{nom} : format {fmt_c} en C, {fmt} dans l'assembleur"


def test_les_codes_absents_de_l_assembleur_sont_inutilises_en_c():
    table = table_c()
    for code in range(256):
        if code in opcodes.PAR_CODE or code in HORS_TRANCHE:
            continue
        assert table[code][1] == "NONE", (
            f"code {code:#04x} : {table[code][0]} défini en C seulement"
        )
