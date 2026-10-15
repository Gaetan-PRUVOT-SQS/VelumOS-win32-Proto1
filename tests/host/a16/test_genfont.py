import fnmatch
import sys
import tomllib
import unittest
from dataclasses import replace
from pathlib import Path
from unittest import mock

from PIL import ImageFont

RACINE = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(RACINE / "tools"))

import genfont_emit as emit  # noqa: E402
import genfont_raster as raster  # noqa: E402
from genfont import SPECS  # noqa: E402

FONTS = RACINE / "third_party" / "fonts"

ASCII = list(range(0x20, 0x7F))
LETTERS_LOW = [0xE0, 0xE2, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEE, 0xEF, 0xF4, 0xF9, 0xFB, 0xFC, 0x153]
EXPECTED = {
    "french": sorted(
        ASCII
        + [0xC0, 0xC2, 0xC7, 0xC8, 0xC9, 0xCA, 0xCE, 0xD4]
        + LETTERS_LOW
        + [0xA0, 0xAB, 0xBB, 0xB0, 0xB7, 0xD7, 0x2013, 0x2014, 0x2019, 0x2026]
    ),
    "display": sorted(
        ASCII
        + [0xC0, 0xC7, 0xC8, 0xC9, 0xCA]
        + LETTERS_LOW
        + [0xA0, 0xAB, 0xBB, 0x2013, 0x2014, 0x2019, 0x2026]
    ),
    "digits": sorted(list(range(0x30, 0x3A)) + [0x20, 0x3A, 0xA0]),
}
SIZES = {"french": 128, "display": 122, "digits": 14}
CHARSET_OF = {
    "ins_bold_24": "display",
    "ins_bold_26": "display",
    "ins_bold_28": "display",
    "ins_semi_120": "digits",
}


def spec_of(ident):
    return next(spec for spec in SPECS if spec.ident == ident)


class CoverageTests(unittest.TestCase):
    def test_codepoints_present_in_open_sans(self):
        data = (FONTS / "OpenSans-Regular.ttf").read_bytes()
        for cp in (0x20, 0x41, 0xE9, 0x152, 0x17F, 0x2026, 0x20AC):
            self.assertNotEqual(raster.cmap_glyph_id(data, cp), 0, hex(cp))

    def test_codepoints_absent(self):
        data = (FONTS / "OpenSans-Regular.ttf").read_bytes()
        for cp in (0x4E2D, 0x1F600, 0x0378):
            self.assertEqual(raster.cmap_glyph_id(data, cp), 0, hex(cp))

    def test_missing_codepoint_stops_generation(self):
        with self.assertRaises(raster.FontError):
            raster.check_coverage(FONTS / "OpenSans-Regular.ttf", [0x41, 0x4E2D])

    def test_charset_is_sorted_and_complete(self):
        cps = raster.charset()
        self.assertEqual(cps, sorted(set(cps)))
        self.assertEqual(len(cps), 329)
        self.assertNotIn(raster.REPLACEMENT, cps)


class ReducedCharsetTests(unittest.TestCase):
    def test_reduced_sets_are_the_ones_of_the_brief(self):
        for name, expected in EXPECTED.items():
            self.assertEqual(raster.charset(name), expected, name)
            self.assertEqual(len(expected) + 1, SIZES[name], name)

    def test_reduced_sets_exclude_replacement_and_soft_hyphen(self):
        for name in EXPECTED:
            cps = raster.charset(name)
            self.assertNotIn(raster.REPLACEMENT, cps, name)
            self.assertNotIn(0xAD, cps, name)

    def test_charset_assigned_to_each_font(self):
        self.assertEqual(len(SPECS), 21)
        for spec in SPECS:
            self.assertEqual(spec.charset, CHARSET_OF.get(spec.ident, "french"), spec.ident)

    def test_unknown_charset_is_refused(self):
        with self.assertRaises(raster.FontError):
            raster.charset("latin")


class RasterTests(unittest.TestCase):
    def test_fixed_advance_glyph_is_kept_in_cell(self):
        spec = spec_of("mono")
        left = raster.Glyph(0xC5, 7, 13, -1, 0, 8, b"")
        right = raster.Glyph(0x10F, 8, 10, 1, 3, 8, b"")
        wide = raster.Glyph(0x57, 9, 9, 0, 4, 8, b"")
        self.assertEqual(raster.keep_in_cell(spec, left).xoff, 0)
        self.assertEqual(raster.keep_in_cell(spec, right).xoff, 0)
        with self.assertRaises(raster.FontError):
            raster.keep_in_cell(spec, wide)

    def test_proportional_font_is_left_alone(self):
        glyph = raster.Glyph(0x66, 5, 9, -1, 3, 4, b"")
        self.assertIs(raster.keep_in_cell(spec_of("ui"), glyph), glyph)

    def test_soft_hyphen_is_the_hyphen(self):
        spec = replace(spec_of("ui"), charset="full")
        glyphs = {glyph.cp: glyph for glyph in raster.rasterize(spec, FONTS)}
        hyphen, soft = glyphs[0x2D], glyphs[0xAD]
        shape = ("bits", "width", "height", "xoff", "yoff", "advance")
        for field in shape:
            self.assertEqual(getattr(soft, field), getattr(hyphen, field), field)

    def test_every_font_fits_its_cell(self):
        for spec in SPECS:
            glyphs = raster.rasterize(spec, FONTS)
            expected = EXPECTED[spec.charset]
            self.assertEqual(len(glyphs), SIZES[spec.charset], spec.ident)
            self.assertEqual([glyph.cp for glyph in glyphs[:-1]], expected, spec.ident)
            self.assertEqual(glyphs[-1].cp, raster.REPLACEMENT)

    def test_mono_ink_stays_inside_its_8_pixel_cell(self):
        for glyph in raster.rasterize(spec_of("mono"), FONTS):
            self.assertGreaterEqual(glyph.xoff, 0, hex(glyph.cp))
            self.assertLessEqual(glyph.xoff + glyph.width, 8, hex(glyph.cp))


def raw_glyphs(spec):
    font = ImageFont.truetype(
        str(FONTS / spec.source), spec.size, layout_engine=ImageFont.Layout.BASIC
    )
    if spec.weight is not None:
        font.set_variation_by_axes(raster.weight_axes(font, spec.weight))
    return {cp: raster.raster_one(font, cp, spec) for cp in raster.charset(spec.charset)}


class ClippingTests(unittest.TestCase):
    def test_canvas_never_clips_ink(self):
        wide = {"CANVAS": 480, "ORIGIN_X": 120, "ORIGIN_Y": 360}
        for spec in SPECS:
            normal = raw_glyphs(spec)
            with mock.patch.multiple(raster, **wide):
                reference = raw_glyphs(spec)
            for cp, glyph in normal.items():
                self.assertEqual(glyph, reference[cp], f"{spec.ident} U+{cp:04X}")

    def test_clock_digits_have_the_mockup_height(self):
        glyphs = {glyph.cp: glyph for glyph in raster.rasterize(spec_of("ins_semi_120"), FONTS)}
        for cp in range(0x30, 0x3A):
            self.assertGreaterEqual(glyphs[cp].height, 84, hex(cp))
            self.assertLessEqual(glyphs[cp].height, 90, hex(cp))


def reuse_annotations():
    data = tomllib.loads((RACINE / "REUSE.toml").read_text(encoding="utf-8"))
    return data["annotations"]


def annotation_for(path, annotations):
    found = None
    for entry in annotations:
        paths = entry["path"] if isinstance(entry["path"], list) else [entry["path"]]
        if (
            any(fnmatch.fnmatchcase(path, pattern) for pattern in paths)
            and entry.get("precedence") == "override"
        ):
            found = entry
    return found


class LicenceTests(unittest.TestCase):
    def test_generated_font_keeps_the_licence_of_its_source(self):
        annotations = reuse_annotations()
        for spec in SPECS:
            source = annotation_for(f"third_party/fonts/{spec.source}", annotations)
            generated = annotation_for(f"lib/font/gen/font_{spec.ident}.c", annotations)
            self.assertIsNotNone(source, spec.source)
            self.assertIsNotNone(generated, spec.ident)
            self.assertIn(
                source["SPDX-License-Identifier"], generated["SPDX-License-Identifier"], spec.ident
            )
            holders = generated["SPDX-FileCopyrightText"]
            self.assertIn(source["SPDX-FileCopyrightText"], holders, spec.ident)

    def test_notice_names_every_source_font(self):
        notice = (FONTS / "NOTICE.txt").read_text(encoding="utf-8")
        for name in sorted({spec.source for spec in SPECS}):
            self.assertIn(name, notice)


class EmitTests(unittest.TestCase):
    def test_identical_bitmaps_share_storage(self):
        glyphs = [
            raster.Glyph(0x41, 1, 1, 0, 0, 2, b"\x80"),
            raster.Glyph(0x42, 1, 1, 0, 0, 2, b"\x80"),
            raster.Glyph(0x43, 1, 1, 0, 0, 2, b"\x40"),
        ]
        blob, offsets = emit.pack_blob(glyphs)
        self.assertEqual(blob, b"\x80\x40")
        self.assertEqual(offsets, {0x41: 0, 0x42: 0, 0x43: 1})

    def test_output_is_sorted_by_codepoint(self):
        spec = spec_of("ui")
        glyphs = list(reversed(raster.rasterize(spec, FONTS)))
        text, _ = emit.emit_c(spec, glyphs)
        cps = [
            int(line.split(",")[0].strip("\t{"))
            for line in text.splitlines()
            if line.startswith("\t{")
        ]
        self.assertEqual(cps, sorted(cps))
        self.assertEqual(cps[-1], raster.REPLACEMENT)

    def test_output_has_offsets_and_no_pointer_per_glyph(self):
        spec = spec_of("ui")
        text, blob_size = emit.emit_c(spec, raster.rasterize(spec, FONTS))
        rows = [line for line in text.splitlines() if line.startswith("\t{")]
        self.assertNotIn("&", text)
        self.assertNotIn("NULL", text)
        for row in rows:
            fields = [int(field) for field in row.strip("\t{},").split(",")]
            self.assertEqual(len(fields), 7)
            stride = (fields[1] + 7) // 8
            self.assertLessEqual(fields[6] + fields[2] * stride, blob_size)
        self.assertIn(f"g_ui_glyphs, g_ui_bits, {blob_size}}};", text)

    def test_blob_larger_than_16_bit_offsets_is_refused(self):
        spec = spec_of("ui")
        glyphs = [
            raster.Glyph(0x100 + k, 8, 255, 0, 0, 8, bytes([k % 256, k // 256]) * 127 + b"\x01")
            for k in range(258)
        ]
        with self.assertRaises(raster.FontError):
            emit.emit_c(spec, glyphs)


if __name__ == "__main__":
    unittest.main()
