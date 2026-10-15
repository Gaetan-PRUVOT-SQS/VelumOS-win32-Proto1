#!/usr/bin/env python3
import argparse
import itertools
import random
import subprocess
import sys

INTERESTING = (0x00, 0x41, 0x7F, 0x80, 0x8F, 0x90, 0x9F, 0xA0, 0xBF, 0xC0, 0xC2, 0xDF, 0xE0)
CHUNK = 1_000_000


def records_small():
    for first in range(256):
        yield bytes([first])
    for first, second in itertools.product(range(256), repeat=2):
        yield bytes([first, second])


def records_three(quick):
    firsts = (0xE0, 0xE1, 0xED, 0xEE, 0xF0, 0xF4, 0xC2, 0x80) if quick else range(0x80, 256)
    for first, second, third in itertools.product(firsts, range(256), range(256)):
        yield bytes([first, second, third])


def records_four(quick):
    firsts = (0xF0, 0xF4) if quick else range(0xF0, 0xF5)
    thirds = INTERESTING if quick else range(256)
    for first, second, third, fourth in itertools.product(
        firsts, range(256), thirds, (0x7F, 0x80, 0xBF, 0xC0)
    ):
        yield bytes([first, second, third, fourth])


def records_random(seed, count):
    rng = random.Random(seed)
    for _ in range(count):
        yield bytes(rng.choice(INTERESTING + (0xF4, 0xF5, 0xFF)) for _ in range(rng.randint(1, 8)))
        yield bytes(rng.randrange(256) for _ in range(rng.randint(1, 8)))


def chunks(records):
    batch = []
    for record in records:
        batch.append(record)
        if len(batch) == CHUNK:
            yield batch
            batch = []
    if batch:
        yield batch


def run_driver(driver, batch):
    stream = b"".join(bytes([len(record)]) + record for record in batch)
    done = subprocess.run([driver], input=stream, capture_output=True, check=True)
    return done.stdout


def verify(batch, output):
    position = 0
    bad = 0
    for record in batch:
        count = output[position]
        got = output[position + 1 : position + 1 + 4 * count]
        want = record.decode("utf-8", "replace").encode("utf-32-le")
        if got != want:
            bad += 1
            if bad <= 5:
                print(f"ECART {record.hex()} : obtenu {got.hex()} attendu {want.hex()}")
        position += 1 + 4 * count
    if position != len(output):
        raise SystemExit("sortie du pilote de taille inattendue")
    return bad


def main(argv):
    parser = argparse.ArgumentParser(description="decodeur UTF-8 contre CPython")
    parser.add_argument("driver")
    parser.add_argument("--quick", action="store_true")
    parser.add_argument("--seed", type=int, default=0x16A16)
    args = parser.parse_args(argv)
    sources = (
        records_small(),
        records_three(args.quick),
        records_four(args.quick),
        records_random(args.seed, 50_000 if args.quick else 300_000),
    )
    total = 0
    bad = 0
    for batch in chunks(itertools.chain.from_iterable(sources)):
        bad += verify(batch, run_driver(args.driver, batch))
        total += len(batch)
    print(
        f"crosscheck utf-8 : {total} sequences comparees a CPython, {bad} ecart(s), "
        f"graine {args.seed}"
    )
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
