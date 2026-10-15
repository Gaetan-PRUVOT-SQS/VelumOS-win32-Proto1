from genfont_raster import FontError

BYTES_PER_LINE = 16
OFFSET_MAX = 0xFFFF


def pack_blob(glyphs):
    blob = bytearray()
    offsets = {}
    placed = {}
    for glyph in glyphs:
        if not glyph.bits:
            continue
        if glyph.bits not in placed:
            placed[glyph.bits] = len(blob)
            blob.extend(glyph.bits)
        offsets[glyph.cp] = placed[glyph.bits]
    return bytes(blob), offsets


def format_blob(ident, blob):
    lines = [f"static const uint8_t\tg_{ident}_bits[] = {{"]
    for start in range(0, len(blob), BYTES_PER_LINE):
        chunk = blob[start : start + BYTES_PER_LINE]
        lines.append("\t" + ", ".join(f"0x{byte:02x}" for byte in chunk) + ",")
    if not blob:
        lines.append("\t0x00,")
    lines.append("};")
    return lines


def format_glyph(glyph, offsets):
    fields = (glyph.cp, glyph.width, glyph.height, glyph.xoff, glyph.yoff, glyph.advance)
    return "\t{" + ", ".join(str(field) for field in (*fields, offsets.get(glyph.cp, 0))) + "},"


def emit_c(spec, glyphs):
    ordered = sorted(glyphs, key=lambda glyph: glyph.cp)
    blob, offsets = pack_blob(ordered)
    if len(blob) > OFFSET_MAX:
        raise FontError(
            f"{spec.ident} : {len(blob)} octets de bitmaps, décalage sur 16 bits dépassé"
        )
    lines = ['#include "../font_int.h"', ""]
    lines += format_blob(spec.ident, blob)
    lines += ["", f"static const t_glyph\tg_{spec.ident}_glyphs[] = {{"]
    lines += [format_glyph(glyph, offsets) for glyph in ordered]
    lines += ["};", ""]
    height = spec.ascent + spec.descent
    lines.append(
        f"const t_font\tg_font_{spec.ident} = {{{spec.ascent}, {spec.descent}, {height}, "
        f"{len(ordered)}, g_{spec.ident}_glyphs, g_{spec.ident}_bits, {len(blob)}}};"
    )
    return "\n".join(lines) + "\n", len(blob)
