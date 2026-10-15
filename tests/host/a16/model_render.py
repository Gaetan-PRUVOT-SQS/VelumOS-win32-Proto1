import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(RACINE / "tools"))

import genfont_raster as raster  # noqa: E402
from genfont import SPECS  # noqa: E402

FONTS = RACINE / "third_party" / "fonts"
PAPER = 0xFFFFFF
INK = 0x000000
SPACES = (0xA0, 0x202F)
ELLIPSIS = "\u2026"

PAGE_W = 520
PAGE_LINES = 7
PAGE = (
    "Démarrer",
    "Poste de travail",
    "Éteindre l'ordinateur",
    "Fichier  Édition  Affichage  Favoris  Outils  ?",
    "ÀÂÇÉÈÊËÎÏÔÙÛÜŸ àâçéèêëîïôùûüÿ œŒ € « » … – — ‘ ’ “ ”",
    "ĀăĄćČďĐēĘěĞĢīİıŁńŇŌőřŚŞŠţŤūůŰŹŻž ĲĳĸŉŊŋſ",
    "The quick brown fox jumps over the lazy dog 0123456789 (){}[]<>",
)
SCENE_W = 360
SCENE_H = 200
LONG_NAME = "Un très long nom de fichier qui ne tient pas dans la boîte"


class Font:
    def __init__(self, spec):
        glyphs = raster.rasterize(spec, FONTS)
        self.glyphs = {glyph.cp: glyph for glyph in glyphs}
        self.height = spec.ascent + spec.descent

    def glyph(self, cp):
        if cp in self.glyphs:
            return self.glyphs[cp]
        if cp in SPACES:
            return self.glyphs[0x20]
        return self.glyphs[raster.REPLACEMENT]

    def width(self, text):
        return sum(self.glyph(ord(char)).advance for char in text)

    def fit(self, text, limit):
        used = 0
        for count, char in enumerate(text):
            advance = self.glyph(ord(char)).advance
            if advance > limit:
                return count
            limit -= advance
            used = count + 1
        return used


class Canvas:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.pixels = [PAPER] * (width * height)

    def put(self, x, y, color):
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = color

    def fill(self, x, y, w, h, color):
        for row in range(y, y + h):
            for col in range(x, x + w):
                self.put(col, row, color)

    def draw_glyph(self, glyph, x, y, color):
        stride = (glyph.width + 7) // 8
        for row in range(glyph.height):
            for col in range(glyph.width):
                byte = glyph.bits[row * stride + (col >> 3)]
                if (byte >> (7 - (col & 7))) & 1:
                    self.put(x + glyph.xoff + col, y + glyph.yoff + row, color)

    def text(self, font, x, y, string, color=INK):
        for char in string:
            glyph = font.glyph(ord(char))
            self.draw_glyph(glyph, x, y, color)
            x += glyph.advance

    def hash(self):
        value = 14695981039346656037
        for pixel in self.pixels:
            value = ((value ^ (pixel & 0xFFFFFF)) * 1099511628211) % (1 << 64)
        return value


def fonts():
    return [Font(spec) for spec in SPECS]


def page_hash(font):
    height = 8 + PAGE_LINES * (font.height + 2)
    canvas = Canvas(PAGE_W, height)
    for line, string in enumerate(PAGE):
        canvas.text(font, 6, 4 + line * (font.height + 2), string)
    return canvas.hash()


def ellipsis(canvas, font, box, string):
    dots = font.width(ELLIPSIS)
    shown = string
    if font.width(string) > box[2]:
        shown = string[: font.fit(string, box[2] - dots)]
    canvas.text(font, box[0], box[1], shown)
    if shown != string:
        canvas.text(font, box[0] + font.width(shown), box[1], ELLIPSIS)


def button(canvas, font, box, label):
    x, y, w, h = box
    canvas.fill(x, y, w, h, 0x003C74)
    canvas.fill(x + 1, y + 1, w - 2, h - 2, 0xF4F3EE)
    left = x + (w - font.width(label)) // 2
    canvas.text(font, left, y + (h - font.height) // 2, label)


def scene_hash(by_name):
    ui, bold = by_name["ui"], by_name["ui_bold"]
    title, mono = by_name["title"], by_name["mono"]
    canvas = Canvas(SCENE_W, SCENE_H)
    canvas.fill(0, 0, SCENE_W, SCENE_H, 0xECE9D8)
    canvas.fill(0, 0, SCENE_W, 30, 0x0058EE)
    canvas.text(title, 9, 8, "Poste de travail", 0x0A246A)
    canvas.text(title, 8, 7, "Poste de travail", 0xFFFFFF)
    canvas.text(ui, 8, 36, "Fichier  Édition  Affichage  Favoris  Outils  ?")
    canvas.fill(8, 58, 150, 15, 0x316AC5)
    canvas.text(ui, 12, 59, "Éteindre l'ordinateur", 0xFFFFFF)
    canvas.text(ui, 12, 76, "Démarrer")
    ellipsis(canvas, ui, (12, 93, 140, 15), LONG_NAME)
    canvas.text(bold, 12, 116, "Paramètres d'affichage")
    button(canvas, ui, (12, 138, 75, 23), "Annuler")
    canvas.fill(180, 58, 172, 130, 0x000000)
    canvas.text(mono, 184, 62, "C:\\> echo « Salut »", 0xC0C0C0)
    canvas.text(mono, 184, 78, "« Salut » … œuvre €", 0xC0C0C0)
    canvas.text(mono, 184, 94, "C:\\> _", 0xFFFFFF)
    return canvas.hash()


def main():
    loaded = fonts()
    by_name = {spec.ident: font for spec, font in zip(SPECS, loaded)}
    for spec, font in zip(SPECS, loaded):
        print(f"page {spec.ident} 0x{page_hash(font):016x}")
    print(f"scene 0x{scene_hash(by_name):016x}")


if __name__ == "__main__":
    main()
