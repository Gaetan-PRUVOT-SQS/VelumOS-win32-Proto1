#!/usr/bin/env python3
import argparse
import hashlib
import platform
import sys
from pathlib import Path

import PIL
from genfont_emit import emit_c
from genfont_preview import write_sheet
from genfont_raster import FontError, FontSpec, rasterize
from PIL import features

ROOT = Path(__file__).resolve().parent.parent
SPECS = (
    FontSpec("ui", "OpenSans-Regular.ttf", 11, 12, 3, charset="french"),
    FontSpec("ui_bold", "OpenSans-ExtraBold.ttf", 11, 12, 3, charset="french"),
    FontSpec("title", "OpenSans-Bold.ttf", 13, 13, 3, charset="french"),
    FontSpec(
        "mono",
        "NotoSansMono-Variable.ttf",
        13,
        13,
        3,
        fixed_advance=8,
        weight=400,
        charset="french",
    ),
    FontSpec("ins_11", "InstrumentSans-Variable.ttf", 11, 11, 3, weight=400, charset="french"),
    FontSpec("ins_12", "InstrumentSans-Variable.ttf", 12, 11, 3, weight=400, charset="french"),
    FontSpec("ins_semi_12", "InstrumentSans-Variable.ttf", 12, 11, 3, weight=600, charset="french"),
    FontSpec("ins_13", "InstrumentSans-Variable.ttf", 13, 14, 3, weight=400, charset="french"),
    FontSpec("ins_semi_13", "InstrumentSans-Variable.ttf", 13, 14, 3, weight=600, charset="french"),
    FontSpec("ins_bold_13", "InstrumentSans-Variable.ttf", 13, 14, 3, weight=700, charset="french"),
    FontSpec("ins_14", "InstrumentSans-Variable.ttf", 14, 14, 3, weight=400, charset="french"),
    FontSpec("ins_semi_14", "InstrumentSans-Variable.ttf", 14, 14, 3, weight=600, charset="french"),
    FontSpec("ins_bold_14", "InstrumentSans-Variable.ttf", 14, 14, 3, weight=700, charset="french"),
    FontSpec("ins_15", "InstrumentSans-Variable.ttf", 15, 15, 3, weight=400, charset="french"),
    FontSpec("ins_22", "InstrumentSans-Variable.ttf", 22, 22, 5, weight=400, charset="french"),
    FontSpec("ins_semi_22", "InstrumentSans-Variable.ttf", 22, 22, 5, weight=600, charset="french"),
    FontSpec(
        "ins_bold_24", "InstrumentSans-Variable.ttf", 24, 24, 5, weight=700, charset="display"
    ),
    FontSpec(
        "ins_bold_26", "InstrumentSans-Variable.ttf", 26, 26, 6, weight=700, charset="display"
    ),
    FontSpec(
        "ins_bold_28", "InstrumentSans-Variable.ttf", 28, 28, 6, weight=700, charset="display"
    ),
    FontSpec(
        "ins_semi_120", "InstrumentSans-Variable.ttf", 120, 89, 1, weight=600, charset="digits"
    ),
    FontSpec(
        "jbm_14",
        "JetBrainsMono-Variable.ttf",
        14,
        14,
        3,
        fixed_advance=8,
        weight=400,
        charset="french",
    ),
)
MANIFEST_NAME = "MANIFEST"


def sha256_of(data):
    return hashlib.sha256(data).hexdigest()


def tool_versions():
    return {
        "python": platform.python_version(),
        "pillow": PIL.__version__,
        "freetype": features.version("freetype2") or "inconnue",
    }


def build_font(spec, fonts_dir):
    glyphs = rasterize(spec, fonts_dir)
    text, blob_size = emit_c(spec, glyphs)
    source = (Path(fonts_dir) / spec.source).read_bytes()
    line = (
        f"font {spec.ident} source={spec.source} source_sha256={sha256_of(source)} "
        f"size={spec.size} weight={spec.weight} ascent={spec.ascent} descent={spec.descent} "
        f"glyphs={len(glyphs)} bits={blob_size} c_sha256={sha256_of(text.encode())}"
    )
    return glyphs, text, line


def build_all(fonts_dir):
    outputs = {}
    manifest = ["genfont 1"] + [f"{name} {value}" for name, value in tool_versions().items()]
    sheets = []
    for spec in SPECS:
        glyphs, text, line = build_font(spec, fonts_dir)
        outputs[f"font_{spec.ident}.c"] = text
        manifest.append(line)
        sheets.append((spec, glyphs))
    outputs[MANIFEST_NAME] = "\n".join(manifest) + "\n"
    return outputs, sheets


def write_outputs(outputs, out_dir):
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    for name, text in outputs.items():
        (out / name).write_text(text, encoding="utf-8", newline="\n")


def differing(outputs, out_dir):
    out = Path(out_dir)
    bad = []
    for name, text in outputs.items():
        path = out / name
        if not path.is_file() or path.read_text(encoding="utf-8") != text:
            bad.append(name)
    return bad


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Rastérise les polices en tableaux C 1 bit")
    parser.add_argument("--fonts", default=str(ROOT / "third_party" / "fonts"))
    parser.add_argument("--out", default=str(ROOT / "lib" / "font" / "gen"))
    parser.add_argument("--check", action="store_true", help="compare sans écrire")
    parser.add_argument("--sheets", metavar="DIR", help="écrit les planches PNG de relecture")
    return parser.parse_args(argv)


def main(argv):
    args = parse_args(argv)
    versions = tool_versions()
    print("genfont : " + ", ".join(f"{k} {v}" for k, v in versions.items()), file=sys.stderr)
    try:
        outputs, sheets = build_all(args.fonts)
    except FontError as error:
        print(f"genfont : {error}", file=sys.stderr)
        return 1
    if args.sheets:
        Path(args.sheets).mkdir(parents=True, exist_ok=True)
        for spec, glyphs in sheets:
            print(f"planche : {write_sheet(spec, glyphs, args.sheets)}", file=sys.stderr)
    if args.check:
        bad = differing(outputs, args.out)
        if bad:
            print("genfont --check : différences : " + ", ".join(bad), file=sys.stderr)
            return 1
        print("genfont --check : sortie identique au dépôt", file=sys.stderr)
        return 0
    write_outputs(outputs, args.out)
    for name, text in outputs.items():
        print(f"{sha256_of(text.encode())}  {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
