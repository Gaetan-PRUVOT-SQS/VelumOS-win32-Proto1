import struct
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

REPLACEMENT = 0xFFFD
EXTRA_CODEPOINTS = (
    0x2013,
    0x2014,
    0x2018,
    0x2019,
    0x201C,
    0x201D,
    0x2026,
    0x2039,
    0x203A,
    0x20AC,
)
SOURCE_ALIAS = {0x00AD: 0x002D}
CANVAS = 192
ORIGIN_X = 32
ORIGIN_Y = 128


@dataclass(frozen=True)
class FontSpec:
    ident: str
    source: str
    size: int
    ascent: int
    descent: int
    fixed_advance: int | None = None
    weight: int | None = None
    charset: str = "full"


@dataclass(frozen=True)
class Glyph:
    cp: int
    width: int
    height: int
    xoff: int
    yoff: int
    advance: int
    bits: bytes


class FontError(Exception):
    pass


DIGIT_CODEPOINTS = tuple(range(0x30, 0x3A)) + (0x20, 0x3A, 0xA0)
ASCII_CODEPOINTS = tuple(range(0x20, 0x7F))
FRENCH_LETTERS = "ÀÂÇÈÉÊÎÔàâçèéêëîïôùûüœ"
DISPLAY_LETTERS = "ÀÇÈÉÊàâçèéêëîïôùûüœ"
FRENCH_SIGNS = (0xA0, 0xAB, 0xBB, 0xB0, 0xB7, 0xD7, 0x2013, 0x2014, 0x2019, 0x2026)
DISPLAY_SIGNS = (0xA0, 0xAB, 0xBB, 0x2013, 0x2014, 0x2019, 0x2026)


def charset(name="full"):
    if name == "digits":
        return sorted(DIGIT_CODEPOINTS)
    if name == "french":
        return sorted(set(ASCII_CODEPOINTS + FRENCH_SIGNS + tuple(map(ord, FRENCH_LETTERS))))
    if name == "display":
        return sorted(set(ASCII_CODEPOINTS + DISPLAY_SIGNS + tuple(map(ord, DISPLAY_LETTERS))))
    if name != "full":
        raise FontError(f"jeu de caractères inconnu : {name}")
    cps = list(range(0x20, 0x7F)) + list(range(0xA0, 0x180)) + list(EXTRA_CODEPOINTS)
    return sorted(set(cps))


def sfnt_table(data, tag):
    count = struct.unpack(">H", data[4:6])[0]
    for index in range(count):
        entry = data[12 + 16 * index : 28 + 16 * index]
        name, _, offset, length = struct.unpack(">4sIII", entry)
        if name == tag:
            return data[offset : offset + length]
    raise FontError(f"table {tag!r} absente")


def lookup_format4(sub, cp):
    seg_x2 = struct.unpack(">H", sub[6:8])[0]
    ends = 14
    starts = ends + seg_x2 + 2
    deltas = starts + seg_x2
    ranges = deltas + seg_x2
    for seg in range(seg_x2 // 2):
        end = struct.unpack(">H", sub[ends + 2 * seg : ends + 2 * seg + 2])[0]
        if cp > end:
            continue
        start = struct.unpack(">H", sub[starts + 2 * seg : starts + 2 * seg + 2])[0]
        if cp < start:
            return 0
        delta = struct.unpack(">h", sub[deltas + 2 * seg : deltas + 2 * seg + 2])[0]
        rng = struct.unpack(">H", sub[ranges + 2 * seg : ranges + 2 * seg + 2])[0]
        if rng == 0:
            return (cp + delta) & 0xFFFF
        where = ranges + 2 * seg + rng + 2 * (cp - start)
        gid = struct.unpack(">H", sub[where : where + 2])[0]
        return (gid + delta) & 0xFFFF if gid else 0
    return 0


def lookup_format12(sub, cp):
    groups = struct.unpack(">I", sub[12:16])[0]
    for index in range(groups):
        start, end, gid = struct.unpack(">III", sub[16 + 12 * index : 28 + 12 * index])
        if start <= cp <= end:
            return gid + cp - start
    return 0


def cmap_glyph_id(data, cp):
    cmap = sfnt_table(data, b"cmap")
    count = struct.unpack(">H", cmap[2:4])[0]
    for index in range(count):
        platform, encoding, offset = struct.unpack(">HHI", cmap[4 + 8 * index : 12 + 8 * index])
        if platform not in (0, 3) or (platform == 3 and encoding not in (1, 10)):
            continue
        sub = cmap[offset:]
        fmt = struct.unpack(">H", sub[:2])[0]
        gid = 0
        if fmt == 4 and cp <= 0xFFFF:
            gid = lookup_format4(sub, cp)
        elif fmt == 12:
            gid = lookup_format12(sub, cp)
        if gid:
            return gid
    return 0


def check_coverage(path, cps):
    data = path.read_bytes()
    missing = [cp for cp in cps if cmap_glyph_id(data, cp) == 0]
    if missing:
        listing = ", ".join(f"U+{cp:04X}" for cp in missing)
        raise FontError(f"{path.name} : points de code absents : {listing}")


def replacement_glyph(spec, cap_height, advance):
    box_width = max(3, min(advance - 2, (spec.size * 5 + 5) // 10))
    canvas = Image.new("1", (box_width, cap_height), 0)
    draw = ImageDraw.Draw(canvas)
    draw.rectangle((0, 0, box_width - 1, cap_height - 1), outline=1)
    return Glyph(
        REPLACEMENT,
        box_width,
        cap_height,
        (advance - box_width) // 2,
        spec.ascent - cap_height,
        advance,
        canvas.tobytes(),
    )


def integral_advance(font, char, spec):
    value = font.getlength(char, mode="1")
    if value != int(value):
        raise FontError(f"chasse non entière pour U+{ord(char):04X} : {value}")
    if spec.fixed_advance is None:
        return int(value)
    if abs(value - spec.fixed_advance) > 1:
        raise FontError(f"{spec.ident} U+{ord(char):04X} : chasse {value} loin de la chasse fixe")
    return spec.fixed_advance


def raster_one(font, cp, spec):
    char = chr(SOURCE_ALIAS.get(cp, cp))
    canvas = Image.new("1", (CANVAS, CANVAS), 0)
    draw = ImageDraw.Draw(canvas)
    draw.fontmode = "1"
    draw.text((ORIGIN_X, ORIGIN_Y), char, font=font, fill=1, anchor="ls")
    advance = integral_advance(font, char, spec)
    box = canvas.getbbox()
    if box is None:
        return Glyph(cp, 0, 0, 0, 0, advance, b"")
    left, top, right, bottom = box
    return Glyph(
        cp,
        right - left,
        bottom - top,
        left - ORIGIN_X,
        spec.ascent - (ORIGIN_Y - top),
        advance,
        canvas.crop(box).tobytes(),
    )


def keep_in_cell(spec, glyph):
    if spec.fixed_advance is None:
        return glyph
    if glyph.width > spec.fixed_advance:
        raise FontError(f"{spec.ident} U+{glyph.cp:04X} : encre plus large que la cellule")
    xoff = min(max(glyph.xoff, 0), spec.fixed_advance - glyph.width)
    return Glyph(glyph.cp, glyph.width, glyph.height, xoff, glyph.yoff, glyph.advance, glyph.bits)


def check_cell(spec, glyph):
    if glyph.advance <= 0 or glyph.advance > 255:
        raise FontError(f"{spec.ident} U+{glyph.cp:04X} : chasse {glyph.advance} hors 1..255")
    if spec.fixed_advance is not None and glyph.advance != spec.fixed_advance:
        raise FontError(f"{spec.ident} U+{glyph.cp:04X} : chasse {glyph.advance} non fixe")
    if glyph.height and not 0 <= glyph.yoff <= spec.ascent + spec.descent - glyph.height:
        raise FontError(
            f"{spec.ident} U+{glyph.cp:04X} : sort de la cellule "
            f"(yoff {glyph.yoff}, h {glyph.height}, cellule {spec.ascent + spec.descent})"
        )
    if not -128 <= glyph.xoff <= 127 or glyph.width > 255:
        raise FontError(f"{spec.ident} U+{glyph.cp:04X} : décalage ou largeur hors limites")


def weight_axes(font, weight):
    axes = font.get_variation_axes()
    names = [axis["name"] for axis in axes]
    if names.count(b"Weight") != 1:
        raise FontError(f"axe de graisse introuvable : {names}")
    values = [axis["default"] for axis in axes]
    values[names.index(b"Weight")] = weight
    return values


def rasterize(spec, fonts_dir):
    path = Path(fonts_dir) / spec.source
    cps = charset(spec.charset)
    check_coverage(path, [SOURCE_ALIAS.get(cp, cp) for cp in cps])
    font = ImageFont.truetype(str(path), spec.size, layout_engine=ImageFont.Layout.BASIC)
    if spec.weight is not None:
        font.set_variation_by_axes(weight_axes(font, spec.weight))
    glyphs = [keep_in_cell(spec, raster_one(font, cp, spec)) for cp in cps]
    cap = next(g for g in glyphs if g.cp == ord("0" if spec.charset == "digits" else "H"))
    advance = glyphs[cps.index(ord("0"))].advance
    glyphs.append(replacement_glyph(spec, cap.height, advance))
    for glyph in glyphs:
        check_cell(spec, glyph)
    return glyphs
