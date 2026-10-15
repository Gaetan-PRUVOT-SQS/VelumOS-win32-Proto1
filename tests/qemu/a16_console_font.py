import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))

from genfont import SPECS  # noqa: E402
from genfont_raster import REPLACEMENT, rasterize  # noqa: E402

NAME = "a16 : la console noyau affiche la police mono pixel pour pixel"
CMDLINE = "verbose init=/aucun"
CELL_W = 8
CELL_H = 16
FOREGROUND = (0xC0, 0xC0, 0xC0)
LINES = 12


def cell_bitmap(glyph):
    rows = [0] * CELL_H
    stride = (glyph.width + 7) // 8
    for row in range(glyph.height):
        for col in range(glyph.width):
            if (glyph.bits[row * stride + col // 8] >> (7 - col % 8)) & 1:
                rows[glyph.yoff + row] |= 1 << (CELL_W - 1 - glyph.xoff - col)
    return tuple(rows)


def glyph_table():
    spec = next(item for item in SPECS if item.ident == "mono")
    glyphs = rasterize(spec, RACINE / "third_party" / "fonts")
    return {cell_bitmap(glyph): chr(glyph.cp) for glyph in glyphs if glyph.cp != REPLACEMENT}


def screen_cell(image, col, row):
    rows = []
    for y in range(CELL_H):
        bits = 0
        for x in range(CELL_W):
            pixel = image.pixel(col * CELL_W + x, row * CELL_H + y)
            bits = (bits << 1) | (pixel == FOREGROUND)
        rows.append(bits)
    return tuple(rows)


def decode_line(image, row, table):
    text = []
    for col in range(image.largeur // CELL_W):
        cell = screen_cell(image, col, row)
        if not any(cell):
            text.append(" ")
            continue
        assert cell in table, f"cellule ({col}, {row}) absente de la police : {cell}"
        text.append(table[cell])
    return "".join(text).rstrip()


def run(vm):
    vm.attendre(r"VelumOS boot ok", delai=60)
    image = vm.capture("a16_console.ppm")
    table = glyph_table()
    serie = vm.serie()
    lines = [decode_line(image, row, table) for row in range(LINES)]
    lines = [line for line in lines if line]
    assert lines, "aucun texte sur l'ecran"
    for line in lines:
        assert line[:60] in serie, f"ligne d'ecran absente du journal serie : {line!r}"
