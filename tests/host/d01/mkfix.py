#!/usr/bin/env python3
"""Fabrique les fixtures du lot d01 avec zipfile et zlib (hors dépôt)."""

import io
import random
import struct
import sys
import warnings
import zipfile
import zlib
from pathlib import Path

E_INVAL = -22
E_RANGE = -34
E_NOTSUP = -95
ORDRE = [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]


def rec(code, a, b=b""):
    return struct.pack("<iII", code, len(a), len(b)) + a + b


def deflate(data, niveau, strategie=zlib.Z_DEFAULT_STRATEGY, coupe=False):
    c = zlib.compressobj(niveau, zlib.DEFLATED, -15, 9, strategie)
    if not coupe:
        return c.compress(data) + c.flush()
    m = len(data) // 2
    out = c.compress(data[:m]) + c.flush(zlib.Z_FULL_FLUSH)
    return out + c.compress(data[m:]) + c.flush()


class Bits:
    def __init__(self):
        self.v = 0
        self.n = 0

    def put(self, val, n):
        self.v |= val << self.n
        self.n += n

    def code(self, val, n):
        for i in range(n - 1, -1, -1):
            self.put((val >> i) & 1, 1)

    def octets(self, marge=8):
        return self.v.to_bytes((self.n + 7) // 8 + marge, "little")


def dyn(cl):
    b = Bits()
    b.put(1, 1)
    b.put(2, 2)
    b.put(1, 5)
    b.put(0, 5)
    b.put(15, 4)
    for s in ORDRE:
        b.put(cl.get(s, 0), 3)
    return b


def fixe():
    b = Bits()
    b.put(1, 1)
    b.put(1, 2)
    return b


def flux_refuses():
    out = [b"", b"\x07", b"\x01\x01\x00\x00\x00", b"\x01\x05\x00\xfa\xffa"]
    out.append(b"\x03\x02")
    out.append(dyn({0: 1, 1: 1, 2: 1}).octets())
    out.append(dyn({0: 1}).octets())
    b = dyn({1: 1, 2: 1})
    b.put(0, 259)
    out.append(b.octets())
    b = dyn({0: 1, 1: 2, 2: 2})
    b.code(3, 2)
    b.put(0, 255)
    b.code(3, 2)
    b.put(0, 2)
    out.append(b.octets())
    b = dyn({0: 1, 1: 1})
    b.put(3, 2)
    b.put(0, 257)
    out.append(b.octets())
    b = dyn({0: 1, 16: 1})
    b.put(1, 1)
    b.put(0, 2)
    out.append(b.octets())
    b = dyn({0: 1, 18: 1})
    b.put(0xFF, 8)
    b.put(0xFF, 8)
    out.append(b.octets())
    b = fixe()
    b.code(0b11000110, 8)
    out.append(b.octets(0))
    b = fixe()
    b.code(0x30 + 97, 8)
    b.code(1, 7)
    b.code(30, 5)
    out.append(b.octets(0))
    return b"".join(rec(E_INVAL, s) for s in out)


def flux_bons(alea):
    mots = [b"velum", b"zip", b"fenetre", b"noyau", b"octet", b" ", b"\n"]
    jeux = [
        b"",
        b"a",
        b"abc" * 5000,
        bytes(range(256)) * 2,
        alea.randbytes(70000),
        b"".join(alea.choice(mots) for _ in range(6000)),
        b"\0" * 3000,
    ]
    strategies = [zlib.Z_FIXED, zlib.Z_HUFFMAN_ONLY, zlib.Z_RLE, zlib.Z_FILTERED]
    out = []
    for d in jeux:
        for niveau in range(10):
            out.append(rec(0, deflate(d, niveau), d))
        for s in strategies:
            out.append(rec(0, deflate(d, 6, s), d))
        out.append(rec(0, deflate(d, 6, coupe=True), d))
        out.append(rec(0, deflate(d, 0, coupe=True), d))
    return b"".join(out)


def archive(entrees, commentaire=b""):
    buf = io.BytesIO()
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        with zipfile.ZipFile(buf, "w") as z:
            z.comment = commentaire
            for nom, data, methode in entrees:
                z.writestr(zipfile.ZipInfo(nom), data, methode, 9)
    return buf.getvalue()


def patch(base, off, fmt, val):
    b = bytearray(base)
    struct.pack_into(fmt, b, off, val)
    return bytes(b)


def cas_archives():
    st, de = zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED
    base = archive([("a.txt", b"bonjour" * 10, de), ("b/c.bin", bytes(range(256)), st)])
    cd = base.find(b"PK\x01\x02")
    cd2 = base.find(b"PK\x01\x02", cd + 4)
    fin = base.rfind(b"PK\x05\x06")
    bons = [
        base,
        archive([]),
        archive([], b"c" * 65535),
        archive([("x", b"y", st)], b"c" * 65535),
        archive([("vide", b"", st), ("n" * 255, b"z" * 99, de)]),
        archive([(f"f{i}", b"x", st) for i in range(300)]),
        archive([(f"f{i}", b"", st) for i in range(4096)]),
    ]
    out = [rec(0, a) for a in bons]
    out.append(rec(E_RANGE, archive([(f"f{i}", b"", st) for i in range(4097)])))
    out.append(rec(E_RANGE, archive([("n" * 256, b"z", st)])))
    nonsup = [
        patch(patch(base, fin + 8, "<H", 0xFFFF), fin + 10, "<H", 0xFFFF),
        patch(base, fin + 16, "<I", 0xFFFFFFFF),
        patch(base, fin + 12, "<I", 0xFFFFFFFF),
        patch(base, fin + 4, "<H", 1),
        patch(base, fin + 6, "<H", 1),
        patch(base, fin + 8, "<H", 1),
        patch(base, cd + 8, "<H", 1),
        patch(base, cd + 8, "<H", 0x40),
        patch(base, 6, "<H", 1),
        patch(base, cd + 10, "<H", 12),
        patch(base, cd + 10, "<H", 99),
        patch(base, cd + 20, "<I", 0xFFFFFFFF),
        patch(base, cd + 24, "<I", 0xFFFFFFFF),
        patch(base, cd + 42, "<I", 0xFFFFFFFF),
        patch(base, cd + 34, "<H", 1),
        archive([("a", b"abc" * 50, zipfile.ZIP_BZIP2)]),
        archive([("a", b"abc" * 50, zipfile.ZIP_LZMA)]),
    ]
    out += [rec(E_NOTSUP, a) for a in nonsup]
    inval = [
        b"",
        base[:21],
        base + b"\0",
        base[1:],
        archive([("d", b"1", st), ("d", b"2", st)]),
        patch(base, 30, "<B", ord("z")),
        patch(base, 26, "<H", 4),
        patch(base, 8, "<H", 0),
        patch(base, 0, "<I", 0x04034B51),
        patch(base, cd, "<I", 0x02014B51),
        patch(base, cd + 20, "<I", 0x00100000),
        patch(base, cd + 42, "<I", 0x7FFFFFFF),
        patch(base, cd + 28, "<H", 0),
        patch(base, cd + 28, "<H", 0xFFF0),
        patch(base, cd2 + 24, "<I", 255),
        patch(base, fin + 12, "<I", struct.unpack_from("<I", base, fin + 12)[0] + 1),
        patch(base, fin + 16, "<I", cd + 1),
        patch(patch(base, fin + 8, "<H", 3), fin + 10, "<H", 3),
        patch(patch(base, fin + 8, "<H", 1), fin + 10, "<H", 1),
        patch(base, fin + 20, "<H", 1),
    ]
    out += [rec(E_INVAL, a) for a in inval]
    return b"".join(out)


def archive_ok(dossier, alea):
    st, de = zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED
    entrees = [
        ("vide", b"", st),
        ("a.txt", b"Bonjour VelumOS\n" * 40, de),
        ("dir/b.bin", bytes(range(256)), st),
        ("n" * 255, alea.randbytes(300) + b"q" * 700, de),
        ("zeros", b"\0" * 3000, de),
    ]
    (dossier / "ok.zip").write_bytes(archive(entrees, b"commentaire de fin"))
    pack = b"".join(rec(m, n.encode(), d) for n, d, m in entrees)
    (dossier / "ok.pack").write_bytes(pack)


def main():
    dossier = Path(sys.argv[1])
    dossier.mkdir(parents=True, exist_ok=True)
    alea = random.Random(0xD01)
    (dossier / "deflate.pack").write_bytes(flux_bons(alea))
    (dossier / "mauvais.pack").write_bytes(flux_refuses())
    (dossier / "cas.pack").write_bytes(cas_archives())
    archive_ok(dossier, alea)


if __name__ == "__main__":
    main()
