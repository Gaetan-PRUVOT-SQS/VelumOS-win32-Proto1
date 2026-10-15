import importlib.util
from pathlib import Path

NAME = "a01 : fault=nmi passe par la pile IST2"
CMDLINE = "selftest exit fault=nmi"

_spec = importlib.util.spec_from_file_location(
    "a01_base", Path(__file__).with_name("a01_fault_div0.py")
)
_base = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_base)


def run(vm):
    _base.verifier_panique(vm, "NMI", "ist2", rip_exact=False, extra=("vec=2 ",))
