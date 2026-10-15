import importlib.util
import re
from pathlib import Path

NAME = "a01 : fault=stack (récursion) tombe sur #DF IST1, pas en triple faute"
CMDLINE = "selftest exit fault=stack"

_spec = importlib.util.spec_from_file_location(
    "a01_base", Path(__file__).with_name("a01_fault_div0.py")
)
_base = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_base)


def run(vm):
    texte = _base.verifier_panique(vm, "#DF", "ist1", rip_exact=False)
    assert "trace : #10 " in texte, texte[-2000:]
    canari = re.search(r"boot: canari noyau, source (rdseed\+rdrand|rdrand|rdseed|tsc)\n", texte)
    assert canari, texte[-2000:]
    garde = re.search(r"fault: page de garde (0x[0-9a-f]+)", texte)
    assert garde and int(garde.group(1), 16) % 4096 == 0, texte[-2000:]
    pile = re.search(r"pile noyau (0x[0-9a-f]+)-(0x[0-9a-f]+) \(32768 o\)", texte)
    assert pile and int(pile.group(1), 16) % 4096 == 0, texte[-2000:]
    assert int(garde.group(1), 16) == int(pile.group(1), 16) - 4096, texte[-2000:]
