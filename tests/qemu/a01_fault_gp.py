import importlib.util
from pathlib import Path

NAME = "a01 : fault=gp donne #GP sur une adresse non canonique"
CMDLINE = "selftest exit fault=gp"

_spec = importlib.util.spec_from_file_location(
    "a01_base", Path(__file__).with_name("a01_fault_div0.py")
)
_base = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_base)


def run(vm):
    _base.verifier_panique(vm, "#GP", "noyau", extra=("rax=0x8000000000000000 ", "err=0 "))
