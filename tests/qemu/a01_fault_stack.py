import importlib.util
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
