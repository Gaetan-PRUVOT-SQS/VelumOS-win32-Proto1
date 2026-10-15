#!/usr/bin/env python3
"""Fabrique les fixtures du lot d02 : tables de chaînes, XML binaire, resources.arsc."""

import struct
import sys
from pathlib import Path

NO = 0xFFFFFFFF
T_REF, T_STR, T_DEC, T_BOOL = 0x01, 0x03, 0x10, 0x12
ATTRS = [0x0101021B, 0x0101021C, 0x0101020C, 0x01010270, 0x01010001, 0x01010003, 0x0101000F]
NAMES = ["versionCode", "versionName", "minSdkVersion", "targetSdkVersion", "label", "name"]
NAMES += ["debuggable", "android", "http://schemas.android.com/apk/res/android", "", "package"]
NAMES += ["manifest", "org.velum.bonjour", "1.0", "uses-sdk", "application", "activity"]
NAMES += [".Main", "texte é\U0001f600"]
TEXTS = ["abc", "é\U0001f600", "x" * 300, ""]


def chunk(kind, head, body=b""):
    hsize = 8 + len(head)
    return struct.pack("<HHI", kind, hsize, hsize + len(body)) + head + body


def len8(n):
    return bytes([n]) if n < 0x80 else bytes([0x80 | (n >> 8), n & 0xFF])


def len16(n):
    if n < 0x8000:
        return struct.pack("<H", n)
    return struct.pack("<HH", 0x8000 | (n >> 16), n & 0xFFFF)


def enc(text, utf8):
    wide = text.encode("utf-16-le")
    if utf8:
        raw = text.encode("utf-8")
        return len8(len(wide) // 2) + len8(len(raw)) + raw + b"\0"
    return len16(len(wide) // 2) + wide + b"\0\0"


def pool(strings, utf8):
    data, offs = b"", b""
    for item in strings:
        offs += struct.pack("<I", len(data))
        data += item if isinstance(item, bytes) else enc(item, utf8)
    data += b"\0" * (-len(data) % 4)
    start = 28 + 4 * len(strings)
    flags = 0x100 if utf8 else 0
    head = struct.pack("<IIIII", len(strings), 0, flags, start, 0)
    return chunk(1, head, offs + data)


def node(kind, ext):
    return chunk(kind, struct.pack("<II", 1, NO), ext)


def attr(name, kind, data, raw=NO):
    return struct.pack("<IIIHBBI", NO, name, raw, 8, 0, kind, data)


def start(name, attrs=(), count=None, size=20):
    count = len(attrs) if count is None else count
    ext = struct.pack("<IIHHHHHH", NO, name, 20, size, count, 0, 0, 0)
    return node(0x102, ext + b"".join(attrs))


def end(name):
    return node(0x103, struct.pack("<II", NO, name))


def text(index):
    return node(0x104, struct.pack("<IHBBI", index, 8, 0, 0, 0))


def resmap():
    return chunk(0x180, b"", b"".join(struct.pack("<I", i) for i in ATTRS))


def xml(parts, utf8=False, head=None):
    head = [pool(NAMES, utf8), resmap()] if head is None else head
    return chunk(3, b"", b"".join(head + parts))


def manifest(utf8=False):
    parts = [node(0x100, struct.pack("<II", 7, 8))]
    top = [attr(0, T_DEC, 1), attr(1, T_STR, 13, 13), attr(10, T_STR, 12, 12)]
    parts += [start(11, top), start(14, [attr(2, T_DEC, 21), attr(3, T_DEC, 33)])]
    parts += [end(14), chunk(0x1FF, b"\0" * 8, b"inconnu!")]
    parts += [start(15, [attr(4, T_REF, 0x7F020000), attr(6, T_BOOL, NO)])]
    parts += [start(16, [attr(5, T_STR, 17, 17)]), text(18), end(16), end(15), end(11)]
    parts += [node(0x101, struct.pack("<II", 7, 8))]
    return xml(parts, utf8)


def deep(levels):
    return xml([start(11)] * levels + [end(11)] * levels)


def bad_xml():
    zero = bytearray(end(11))
    zero[4:8] = b"\0\0\0\0"
    over = bytearray(chunk(0x1FF, b"", b"abcd"))
    over[4:8] = struct.pack("<I", 4096)
    return {
        "bad_end": xml([end(11)]),
        "no_end": xml([start(11)]),
        "zero_chunk": xml([start(11), bytes(zero)]),
        "bad_name": xml([start(999), end(11)]),
        "bad_end_name": xml([start(11), end(999)]),
        "attr_over": xml([start(11, [attr(0, T_DEC, 1)], count=50), end(11)]),
        "attr_small": xml([start(11, [attr(0, T_DEC, 1)], size=8), end(11)]),
        "attr_name": xml([start(11, [attr(999, T_DEC, 1)]), end(11)]),
        "attr_raw": xml([start(11, [attr(0, T_DEC, 1, 999)]), end(11)]),
        "attr_str": xml([start(11, [attr(0, T_STR, 999)]), end(11)]),
        "text_bad": xml([start(11), text(999), end(11)]),
        "two_pools": xml([pool(NAMES, False), start(11), end(11)]),
        "two_maps": xml([resmap(), start(11), end(11)]),
        "no_pool": xml([start(11), end(11)], head=[resmap()]),
        "chunk_over": xml([start(11), end(11), bytes(over)]),
        "short_node": xml([chunk(0x102, b"", b"\0" * 40)]),
    }


def config(lang=b"\0\0"):
    return struct.pack("<IHH2s2s", 64, 0, 0, lang, b"\0\0") + b"\0" * 52


def simple(key, kind, data):
    return struct.pack("<HHIHBBI", 8, 0, key, 8, 0, kind, data)


def bag(key):
    return struct.pack("<HHIII", 16, 1, key, 0, 0)


def rtype(tid, entries, lang=b"\0\0", flags=0):
    offs, data = b"", b""
    for item in entries:
        offs += struct.pack("<I", NO if item is None else len(data))
        data += b"" if item is None else item
    start_at = 84 + 4 * len(entries)
    head = struct.pack("<BBHII", tid, flags, 0, len(entries), start_at) + config(lang)
    return chunk(0x201, head, offs + data)


def spec(tid, count):
    return chunk(0x202, struct.pack("<BBHI", tid, 0, 0, count), b"\0" * 4 * count)


def package(parts, type_off=288):
    tpool = pool(["id", "string"], True)
    kpool = pool([f"cle{i}" for i in range(16)], True)
    name = "org.velum.bonjour".encode("utf-16-le").ljust(256, b"\0")
    offs = struct.pack("<IIIII", type_off, 0, 288 + len(tpool), 0, 0)
    head = struct.pack("<I", 0x7F) + name + offs
    return chunk(0x200, head, tpool + kpool + b"".join(parts))


def table(parts):
    return chunk(2, struct.pack("<I", 1), b"".join(parts))


def entries():
    ref = 0x7F020000
    out = [simple(0, T_STR, 0), simple(1, T_REF, ref), simple(2, T_REF, ref + 3)]
    out += [simple(3, T_REF, ref + 2), simple(4, T_DEC, 42), bag(5)]
    out += [simple(6 + i, T_REF, ref + 7 + i) for i in range(7)]
    out += [simple(13, T_REF, ref), simple(14, T_REF, ref + 6)]
    return out


def arsc(flags=0, with_pool=1, type_off=288):
    glob = pool(["Hello", "Bonjour é"], True)
    base = entries()
    french = [simple(0, T_STR, 1)] + [None] * (len(base) - 1)
    types = [spec(2, len(base)), rtype(2, base, flags=flags)]
    types += [rtype(2, french, b"fr", flags)]
    return table([glob] * with_pool + [package(types, type_off)])


def main():
    out = Path(sys.argv[1])
    lone = [len16(1) + struct.pack("<H", 0xD800) + b"\0\0"]
    lone += [len16(1) + struct.pack("<H", 0xDC00) + b"\0\0"]
    lone += [len16(2) + struct.pack("<HH", 0xD800, 0x41) + b"\0\0"]
    files = {
        "pool_u8.bin": pool(TEXTS, True),
        "pool_u16.bin": pool(TEXTS, False),
        "pool_u16_long.bin": pool(["y" * 0x8000], False),
        "pool_u16_lone.bin": pool(lone, False),
        "manifest.axml": manifest(),
        "manifest_u8.axml": manifest(True),
        "deep_ok.axml": deep(64),
        "deep_ko.axml": deep(65),
        "resources.arsc": arsc(),
        "arsc_nopool.arsc": arsc(with_pool=0),
        "arsc_twopools.arsc": arsc(with_pool=2),
        "arsc_sparse.arsc": arsc(flags=1),
        "arsc_badpkg.arsc": arsc(type_off=1 << 20),
    }
    files.update({f"{k}.axml": v for k, v in bad_xml().items()})
    for name, data in files.items():
        (out / name).write_bytes(data)


if __name__ == "__main__":
    main()
