#!/usr/bin/env python3
import argparse
import hashlib
import subprocess
from pathlib import Path

SHA256_DI = bytes.fromhex("3031300d060960864801650304020105000420")
SHA256_DI_SANS_NULL = bytes.fromhex("302f300b06096086480165030402010420")
SHA1_DI = bytes.fromhex("3021300906052b0e03021a05000414")
OID_RSA = bytes.fromhex("2a864886f70d010101")
OID_PSS = bytes.fromhex("2a864886f70d01010a")
CERT, PARSE, OK = 1, 2, 4
NULL = b"\x05\x00"


def run(*args):
    return subprocess.run(args, check=True, capture_output=True).stdout


def tlv(tag, body, form=None):
    size = len(body)
    if form is not None:
        head = form
    elif size < 0x80:
        head = bytes([size])
    else:
        raw = size.to_bytes((size.bit_length() + 7) // 8, "big")
        head = bytes([0x80 | len(raw)]) + raw
    return bytes([tag]) + head + body


def der_int(value):
    return tlv(2, value.to_bytes(value.bit_length() // 8 + 1, "big"))


def spki(n, e, oid=OID_RSA, params=NULL, unused=0, n_tlv=None, tail=b""):
    alg = tlv(0x30, tlv(6, oid) + params)
    key = tlv(0x30, (n_tlv or der_int(n)) + der_int(e) + tail)
    return tlv(0x30, alg + tlv(3, bytes([unused]) + key))


def read_tlv(buf, pos):
    size = buf[pos + 1]
    pos += 2
    if size & 0x80:
        count = size & 0x7F
        size = int.from_bytes(buf[pos : pos + count], "big")
        pos += count
    return buf[pos : pos + size], pos + size


def private_numbers(pem):
    der = run("openssl", "rsa", "-in", str(pem), "-outform", "DER", "-traditional")
    body, _ = read_tlv(der, 0)
    values = []
    pos = 0
    while len(values) < 4:
        raw, pos = read_tlv(body, pos)
        values.append(int.from_bytes(raw, "big"))
    return values[1], values[2], values[3]


def make_key(work, bits, e):
    pem = work / f"k{bits}e{e}.pem"
    if not pem.exists():
        run(
            "openssl",
            "genpkey",
            "-algorithm",
            "RSA",
            "-pkeyopt",
            f"rsa_keygen_bits:{bits}",
            "-pkeyopt",
            f"rsa_keygen_pubexp:{e}",
            "-out",
            str(pem),
        )
    n, e_read, d = private_numbers(pem)
    pub = run("openssl", "pkey", "-in", str(pem), "-pubout", "-outform", "DER")
    cert = run(
        "openssl",
        "req",
        "-new",
        "-x509",
        "-key",
        str(pem),
        "-subj",
        "/CN=d05",
        "-days",
        "3650",
        "-sha256",
        "-outform",
        "DER",
    )
    assert e_read == e and pub == spki(n, e)
    return {"n": n, "e": e, "d": d, "k": bits // 8, "spki": pub, "cert": cert, "pem": pem}


def emsa(k, digest, info=SHA256_DI):
    tail = info + digest
    return b"\x00\x01" + b"\xff" * (k - 3 - len(tail)) + b"\x00" + tail


def sign_block(block, key):
    value = pow(int.from_bytes(block, "big"), key["d"], key["n"])
    return value.to_bytes(key["k"], "big")


def openssl_sign(work, key, msg):
    path = work / "message.bin"
    path.write_bytes(msg)
    sig = run("openssl", "dgst", "-sha256", "-sign", str(key["pem"]), str(path))
    assert sig == sign_block(emsa(key["k"], hashlib.sha256(msg).digest()), key)
    return sig


def flip(data, bit):
    out = bytearray(data)
    out[bit // 8] ^= 1 << (bit % 8)
    return bytes(out)


def cube_root_ceil(value):
    low, high = 0, 1 << (value.bit_length() // 3 + 2)
    while low < high:
        mid = (low + high) // 2
        if mid**3 < value:
            low = mid + 1
        else:
            high = mid
    return low


def forge_e3(key, digest):
    k = key["k"]
    head = b"\x00\x01" + b"\xff" * 8 + b"\x00" + SHA256_DI + digest
    root = cube_root_ceil(int.from_bytes(head, "big") << (8 * (k - len(head))))
    cube = root**3
    assert cube < key["n"] and cube.to_bytes(k, "big")[: len(head)] == head
    return root.to_bytes(k, "big")


def sig_plus_modulus(key):
    k, n = key["k"], key["n"]
    for i in range(400):
        digest = hashlib.sha256(f"plus {i}".encode()).digest()
        value = int.from_bytes(sign_block(emsa(k, digest), key), "big") + n
        if value < 1 << (8 * k):
            return value.to_bytes(k, "big"), digest
    return None


def positives(work, keys):
    rows = []
    for (bits, e), key in keys.items():
        msg = f"VelumOS d05 {bits} {e}".encode()
        sig = openssl_sign(work, key, msg)
        digest = hashlib.sha256(msg).digest()
        name = f"pos-{bits}-e{e}"
        rows.append((name + "-cert", key["cert"], sig, digest, CERT | PARSE | OK, msg))
        rows.append((name + "-spki", key["spki"], sig, digest, PARSE | OK, msg))
    return rows


def sig_negatives(key, sig, digest, msg):
    n, k, good = key["n"], key["k"], key["spki"]
    block = emsa(k, digest)
    short = b"\x00\x01" + b"\xff" * 8 + b"\x00" + SHA256_DI + digest
    junk = b"\x00\x01" + b"\xff" * (k - 70) + b"\x00" + SHA256_DI + digest + b"\xa5" * 16
    sha384 = SHA256_DI[:14] + b"\x02" + SHA256_DI[15:]
    blocks = {
        "bourrage-court": b"\x00" * (k - len(short)) + short,
        "bourrage-non-ff": block[:10] + b"\xfe" + block[11:],
        "bourrage-zero": block[:10] + b"\x00" + block[11:],
        "type-bloc-02": b"\x00\x02" + block[2:],
        "premier-octet": b"\x01" + block[1:],
        "dechets-apres-condensat": junk,
        "digestinfo-sha1": emsa(k, hashlib.sha1(msg).digest(), SHA1_DI),
        "digestinfo-oid-sha384": emsa(k, digest, sha384),
        "digestinfo-sans-null": emsa(k, digest, SHA256_DI_SANS_NULL),
    }
    sigs = {name: sign_block(value, key) for name, value in blocks.items()}
    sigs["sig-bit"] = flip(sig, 100)
    sigs["sig-egale-modulus"] = n.to_bytes(k, "big")
    sigs["sig-max"] = b"\xff" * k
    sigs["sig-longue"] = b"\x00" + sig
    sigs["sig-courte"] = sig[:-1]
    sigs["sig-vide"] = b""
    sigs["contrefacon-e3"] = forge_e3(key, digest)
    rows = [("neg-" + name, good, value, digest, PARSE, b"") for name, value in sigs.items()]
    rows.append(("neg-condensat-bit", good, sig, flip(digest, 5), PARSE, b""))
    rows.append(("neg-modulus-bit", spki(n ^ (1 << 1000), 3), sig, digest, PARSE, b""))
    plus = sig_plus_modulus(key)
    if plus:
        rows.append(("neg-sig-plus-modulus", good, plus[0], plus[1], PARSE, b""))
    return rows


def key_negatives(key, sig, digest):
    n, e, k, good, cert = key["n"], key["e"], key["k"], key["spki"], key["cert"]
    body, _ = read_tlv(good, 0)
    small = int.from_bytes(hashlib.sha512(b"d05").digest() * 2, "big") | 1 | 1 << 1023
    ec = run("openssl", "genpkey", "-algorithm", "EC", "-pkeyopt", "ec_paramgen_curve:P-256")
    ec_pub = subprocess.run(
        ["openssl", "pkey", "-pubout", "-outform", "DER"],
        input=ec,
        check=True,
        capture_output=True,
    ).stdout
    magnitude = n.to_bytes(k, "big")
    keys = {
        "cle-1024": spki(small, 65537),
        "modulus-pair": spki(n - 1, e),
        "modulus-2047-bits": spki(n >> 1 | 1, e),
        "modulus-4104-bits": spki(n | 1 << 4103, e),
        "exposant-pair": spki(n, 4),
        "exposant-1": spki(n, 1),
        "exposant-0": spki(n, 0),
        "exposant-33-bits": spki(n, 1 << 32 | 1),
        "der-longueur-non-minimale": tlv(0x30, body, b"\x83" + len(body).to_bytes(3, "big")),
        "der-longueur-indefinie": tlv(0x30, body + b"\x00\x00", b"\x80"),
        "der-tronque": good[:-1],
        "der-octets-en-trop": good + b"\x00",
        "der-interne-en-trop": spki(n, e, tail=NULL),
        "der-entier-negatif": spki(n, e, n_tlv=tlv(2, magnitude)),
        "der-entier-non-minimal": spki(n, e, n_tlv=tlv(2, b"\x00\x00" + magnitude)),
        "oid-rsassa-pss": spki(n, e, oid=OID_PSS),
        "oid-courbe-elliptique": ec_pub,
        "parametres-absents": spki(n, e, params=b""),
        "bits-inutilises": spki(n, e, unused=1),
    }
    rows = [("neg-" + name, value, sig, digest, 0, b"") for name, value in keys.items()]
    rows.append(("neg-cert-tronque", cert[:-1], sig, digest, CERT, b""))
    rows.append(("neg-cert-octets-en-trop", cert + b"\x00", sig, digest, CERT, b""))
    rows.append(("neg-cert-est-une-cle", good, sig, digest, CERT, b""))
    return rows


def c_array(name, data):
    body = ",".join(str(b) for b in (data or b"\x00"))
    return f"static const uint8_t {name}[] = {{{body}}};\n"


def emit(rows):
    out = ['#include "d05_vec.h"\n\n']
    table = []
    for i, (name, key, sig, digest, flags, msg) in enumerate(rows):
        out.append(c_array(f"k{i}", key))
        out.append(c_array(f"s{i}", sig))
        out.append(c_array(f"m{i}", msg))
        out.append(c_array(f"d{i}", digest))
        sizes = (len(key), len(sig), len(msg))
        fields = f"k{i}, {sizes[0]}, s{i}, {sizes[1]}, m{i}, {sizes[2]}, d{i}, {flags}"
        table.append(f'\t{{"{name}", {fields}}},\n')
    out.append("\nconst t_vec g_vec[] = {\n" + "".join(table) + "};\n")
    out.append(f"const size_t g_vec_count = {len(rows)};\n")
    return "".join(out)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dir", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()
    args.dir.mkdir(parents=True, exist_ok=True)
    sizes = [(2048, 3), (2048, 65537), (3072, 3), (3072, 65537), (4096, 3), (4096, 65537)]
    keys = {size: make_key(args.dir, *size) for size in sizes}
    rows = positives(args.dir, keys)
    base = keys[(2048, 3)]
    _, _, sig, digest, _, msg = rows[0]
    rows += sig_negatives(base, sig, digest, msg)
    rows += key_negatives(base, sig, digest)
    partial = args.out.with_suffix(".partiel")
    partial.write_text(emit(rows), encoding="ascii")
    partial.replace(args.out)


if __name__ == "__main__":
    main()
