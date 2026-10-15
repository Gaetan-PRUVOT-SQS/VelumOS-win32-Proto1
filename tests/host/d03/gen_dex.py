#!/usr/bin/env python3
import argparse
import hashlib
import math
import struct
import zlib
from pathlib import Path

NAME = "Vélum € \U0001f600"
SOURCE = "Demo.java"
OBJECT = "Ljava/lang/Object;"
STRING = "Ljava/lang/String;"
KLASS = "Ljava/lang/Class;"
EXCEPTION = "Ljava/lang/Exception;"
RUNNABLE = "Ljava/lang/Runnable;"
BASE = "Lvelum/Base;"
DERIVED = "Lvelum/Derived;"
TYPES = sorted(
    ["D", "F", "I", "J", "V", "Z", OBJECT, STRING, KLASS, EXCEPTION, RUNNABLE] + [BASE, DERIVED]
)
PROTOS = [("III", "I", True), ("V", "V", False)]
FIELDS = [
    (BASE, "I", "seed"),
    (DERIVED, "J", "BIG"),
    (DERIVED, "I", "COUNT"),
    (DERIVED, "Z", "FLAG"),
    (DERIVED, KLASS, "KLASS"),
    (DERIVED, STRING, "NAME"),
    (DERIVED, STRING, "NOTHING"),
    (DERIVED, "D", "PI"),
    (DERIVED, "F", "RATIO"),
    (DERIVED, "I", "value"),
]
METHODS = [
    (BASE, 1, "<init>"),
    (BASE, 1, "run"),
    (DERIVED, 1, "<init>"),
    (DERIVED, 0, "add"),
    (DERIVED, 1, "run"),
]
SIZES = [4, 4, 12, 8, 8, 32]


def units(text):
    raw = text.encode("utf-16-le")
    return list(struct.unpack(f"<{len(raw) // 2}H", raw))


def mutf8(text):
    out = bytearray()
    for unit in units(text):
        if unit != 0 and unit < 0x80:
            out.append(unit)
        elif unit < 0x800:
            out += bytes([0xC0 | (unit >> 6), 0x80 | (unit & 0x3F)])
        else:
            out.append(0xE0 | (unit >> 12))
            out += bytes([0x80 | ((unit >> 6) & 0x3F), 0x80 | (unit & 0x3F)])
    return bytes(out)


def uleb(value):
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        if not value:
            out.append(byte)
            return bytes(out)
        out.append(byte | 0x80)


def sleb(value):
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        done = (value == 0 and not byte & 0x40) or (value == -1 and byte & 0x40)
        out.append(byte if done else byte | 0x80)
        if done:
            return bytes(out)


def type_list(tid, names):
    return struct.pack("<I", len(names)) + b"".join(struct.pack("<H", tid[name]) for name in names)


def code_item(registers, ins, insns, tries=(), handlers=b""):
    out = struct.pack("<HHHHII", registers, ins, 0, len(tries), 0, len(insns))
    out += struct.pack(f"<{len(insns)}H", *insns)
    if tries:
        out += b"\0" * (len(out) % 4)
        for start, count, handler in tries:
            out += struct.pack("<IHH", start, count, handler)
        out += handlers
    return out


def add_code(exception):
    exc = uleb(exception)
    first = sleb(-1) + exc + uleb(3) + uleb(4)
    second = sleb(1) + exc + uleb(3)
    third = sleb(0) + uleb(4)
    tries = [(0, 2, 1), (2, 1, 1 + len(first))]
    tries.append((3, 1, 1 + len(first) + len(second)))
    insns = [0x0090, 0x0201, 0x000F, 0x0012, 0x000F]
    return code_item(3, 2, insns, tries, uleb(3) + first + second + third)


def class_data(groups):
    out = b"".join(uleb(len(group)) for group in groups)
    for group in groups:
        last = 0
        for idx, *rest in group:
            out += uleb(idx - last) + b"".join(uleb(item) for item in rest)
            last = idx
    return out


def value(kind, raw):
    return bytes([kind | ((len(raw) - 1) << 5)]) + raw


def derived_values(sid, tid):
    out = uleb(8) + value(0x06, b"\xfe") + value(0x04, struct.pack("<h", 300))
    out += bytes([0x3F]) + value(0x18, bytes([tid[KLASS]]))
    out += value(0x17, bytes([sid[NAME]])) + bytes([0x1E])
    out += value(0x11, struct.pack("<d", math.pi))
    return out + value(0x10, struct.pack("<f", 1.5)[2:])


class Data:
    def __init__(self, base):
        self.base = base
        self.raw = bytearray()
        self.marks = {}

    def put(self, kind, blob, aligned=False):
        while aligned and len(self.raw) % 4:
            self.raw.append(0)
        at = self.base + len(self.raw)
        count, first = self.marks.get(kind, (0, at))
        self.marks[kind] = (count + 1, first)
        self.raw += blob
        return at


def fill(data, sid, tid, texts):
    at = {}
    at["params"] = data.put(0x1001, type_list(tid, ["I", "I"]), True)
    at["ifaces"] = data.put(0x1001, type_list(tid, [RUNNABLE]), True)
    simple = code_item(1, 1, [0x000E])
    code = [data.put(0x2001, simple, True) for _ in range(3)]
    code.append(data.put(0x2001, add_code(tid[EXCEPTION]), True))
    code.append(data.put(0x2001, simple, True))
    at["strings"] = [
        data.put(0x2002, uleb(len(units(text))) + mutf8(text) + b"\0") for text in texts
    ]
    base = [[(0, 0x9)], [], [(0, 0x10001, code[0])], [(1, 0x1, code[1])]]
    derived = [[(i, 0x19) for i in range(1, 9)], [(9, 0x2)]]
    derived.append([(2, 0x10001, code[2])])
    derived.append([(3, 0x1, code[3]), (4, 0x1, code[4])])
    at["base"] = data.put(0x2000, class_data(base))
    at["derived"] = data.put(0x2000, class_data(derived))
    at["base_values"] = data.put(0x2005, uleb(1) + value(0x04, b"\xfe"))
    at["derived_values"] = data.put(0x2005, derived_values(sid, tid))
    return at


def ids(at, sid, tid):
    out = b"".join(struct.pack("<I", off) for off in at["strings"])
    out += b"".join(struct.pack("<I", sid[name]) for name in TYPES)
    for shorty, ret, has_params in PROTOS:
        params = at["params"] if has_params else 0
        out += struct.pack("<III", sid[shorty], tid[ret], params)
    for owner, kind, name in FIELDS:
        out += struct.pack("<HHI", tid[owner], tid[kind], sid[name])
    for owner, proto, name in METHODS:
        out += struct.pack("<HHI", tid[owner], proto, sid[name])
    src = sid[SOURCE]
    out += struct.pack("<8I", tid[BASE], 0x1, tid[OBJECT], 0, src, 0, at["base"], at["base_values"])
    return out + struct.pack(
        "<8I",
        tid[DERIVED],
        0x11,
        tid[BASE],
        at["ifaces"],
        src,
        0,
        at["derived"],
        at["derived_values"],
    )


def map_list(data, counts, offsets):
    while len(data.raw) % 4:
        data.raw.append(0)
    map_off = data.base + len(data.raw)
    entries = [(0, 1, 0)]
    entries += [(k + 1, counts[k], offsets[k]) for k in range(6)]
    marks = sorted(data.marks.items(), key=lambda item: item[1][1])
    entries += [(kind, count, first) for kind, (count, first) in marks]
    entries.append((0x1000, 1, map_off))
    data.raw += struct.pack("<I", len(entries))
    for kind, count, first in entries:
        data.raw += struct.pack("<HHII", kind, 0, count, first)
    return map_off


def build():
    names = {NAME, SOURCE, "a\0b"} | set(TYPES)
    names |= {proto[0] for proto in PROTOS}
    names |= {field[2] for field in FIELDS} | {method[2] for method in METHODS}
    texts = sorted(names, key=units)
    sid = {text: i for i, text in enumerate(texts)}
    tid = {text: i for i, text in enumerate(TYPES)}
    counts = [len(texts), len(TYPES), len(PROTOS), len(FIELDS), len(METHODS), 2]
    offsets = [0x70]
    for count, size in zip(counts, SIZES, strict=True):
        offsets.append(offsets[-1] + count * size)
    data = Data(offsets[6])
    at = fill(data, sid, tid, texts)
    map_off = map_list(data, counts, offsets)
    size = data.base + len(data.raw)
    tail = struct.pack("<6I", size, 0x70, 0x12345678, 0, 0, map_off)
    for count, offset in zip(counts, offsets, strict=False):
        tail += struct.pack("<II", count, offset)
    tail += struct.pack("<II", len(data.raw), data.base)
    tail += ids(at, sid, tid) + bytes(data.raw)
    signed = hashlib.sha1(tail).digest() + tail
    return b"dex\n035\0" + struct.pack("<I", zlib.adler32(signed)) + signed


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sortie", required=True, type=Path)
    args = parser.parse_args()
    args.sortie.parent.mkdir(parents=True, exist_ok=True)
    args.sortie.write_bytes(build())


if __name__ == "__main__":
    main()
