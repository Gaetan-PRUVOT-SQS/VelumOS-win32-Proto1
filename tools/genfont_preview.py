from pathlib import Path

from genfont_raster import REPLACEMENT
from PIL import Image

SAMPLES = (
    "Démarrer",
    "Poste de travail",
    "Éteindre l'ordinateur",
    "Fichier  Édition  Affichage  Favoris  Outils  ?",
    "ÀÂÇÉÈÊËÎÏÔÙÛÜŸ àâçéèêëîïôùûüÿ œŒ € « » … – — ‘ ’ “ ”",
    "ĀăĄćČďĐēĘěĞĢīİıŁńŇŌőŒřŚŞŠţŤūůŰŹŻž ĲĳĸŉŊŋſ",
    "The quick brown fox jumps over the lazy dog 0123456789 (){}[]<>",
)
SCALE = 3
GRID_COLUMNS = 32
INK = 0
PAPER = 255
GUIDE = 215


def pixel_set(glyph, col, row):
    stride = (glyph.width + 7) // 8
    return (glyph.bits[row * stride + col // 8] >> (7 - col % 8)) & 1


def put_glyph(image, glyph, left, top):
    for row in range(glyph.height):
        for col in range(glyph.width):
            if pixel_set(glyph, col, row):
                image.putpixel((left + glyph.xoff + col, top + glyph.yoff + row), INK)


def draw_line(image, table, spec, origin, text):
    pen, top = origin
    for char in text:
        glyph = table.get(ord(char), table[REPLACEMENT])
        put_glyph(image, glyph, pen, top)
        pen += glyph.advance
    return pen


def text_width(table, text):
    return sum(table.get(ord(char), table[REPLACEMENT]).advance for char in text)


def sample_block(table, spec):
    line_height = spec.ascent + spec.descent + 3
    width = max(text_width(table, text) for text in SAMPLES) + 8
    image = Image.new("L", (width, line_height * len(SAMPLES) + 4), PAPER)
    for index, text in enumerate(SAMPLES):
        draw_line(image, table, spec, (4, 2 + index * line_height), text)
    return image


def grid_block(glyphs, spec):
    cell_w = max(glyph.advance for glyph in glyphs) + 3
    cell_h = spec.ascent + spec.descent + 3
    rows = (len(glyphs) + GRID_COLUMNS - 1) // GRID_COLUMNS
    image = Image.new("L", (cell_w * GRID_COLUMNS + 2, cell_h * rows + 2), PAPER)
    for index, glyph in enumerate(sorted(glyphs, key=lambda item: item.cp)):
        left = 2 + (index % GRID_COLUMNS) * cell_w
        top = 2 + (index // GRID_COLUMNS) * cell_h
        for col in range(cell_w - 1):
            image.putpixel((left + col, top + spec.ascent), GUIDE)
        put_glyph(image, glyph, left, top)
    return image


def write_sheet(spec, glyphs, out_dir):
    table = {glyph.cp: glyph for glyph in glyphs}
    samples = sample_block(table, spec)
    grid = grid_block(glyphs, spec)
    width = max(samples.width, grid.width)
    sheet = Image.new("L", (width, samples.height + grid.height + 6), PAPER)
    sheet.paste(samples, (0, 0))
    sheet.paste(grid, (0, samples.height + 6))
    sheet = sheet.resize((sheet.width * SCALE, sheet.height * SCALE), Image.Resampling.NEAREST)
    path = Path(out_dir) / f"sheet_{spec.ident}.png"
    sheet.save(path)
    return path
