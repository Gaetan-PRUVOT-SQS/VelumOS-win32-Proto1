import sys
from pathlib import Path

NAME = "virtio-blk absent ou legacy : démarrage propre sans disque"
CMDLINE = "selftest exit"

RACINE = Path(__file__).resolve().parent.parent.parent


def _image():
    argv = sys.argv
    dossier = RACINE / "build" / "a11" / "qemu" / "a11_donnees"
    if "--out" in argv and argv.index("--out") + 1 < len(argv):
        dossier = Path(argv[argv.index("--out") + 1]).resolve() / "a11_donnees"
    dossier.mkdir(parents=True, exist_ok=True)
    chemin = dossier / "a11_legacy.img"
    if not chemin.exists():
        chemin.write_bytes(bytes(64 * 512))
    return chemin


class _Legacy:
    def __iter__(self):
        return iter(
            [
                "-drive",
                f"file={_image()},if=none,id=d3,format=raw,readonly=on",
                "-device",
                "virtio-blk-pci,drive=d3,disable-modern=on",
            ]
        )


QEMU_EXTRA = _Legacy()


def run(vm):
    vm.attendre(r"block: virtio-blk legacy .* ignoré", delai=60)
    vm.attendre(r"block: aucun disque virtio")
    vm.attendre(r"\[TEST\] block \.\.\. OK", delai=60)
    vm.attendre(r"SELFTESTS PASS \d+", delai=60)
