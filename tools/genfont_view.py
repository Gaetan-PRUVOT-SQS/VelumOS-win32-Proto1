#!/usr/bin/env python3
import sys
from pathlib import Path

from PIL import Image


def convert(source):
    target = source.with_suffix(".png")
    with Image.open(source) as image:
        image.convert("RGB").save(target)
    return target


def main(argv):
    if not argv:
        print("usage : genfont_view.py fichier.ppm...", file=sys.stderr)
        return 2
    for name in argv:
        print(convert(Path(name)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
