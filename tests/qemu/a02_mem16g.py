import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from a02_mem8g import mesurer  # noqa: E402

NAME = "a02 : mémoire physique avec 16 Gio (RAM haute, tas dimensionné, autotests)"
CMDLINE = "selftest exit"
MEMOIRE = "16G"


def run(vm):
    mesurer(vm, 16, 256)
