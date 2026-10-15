import re

NAME = "a09 : pont PCI-PCI imbrique et MSI de bout en bout (appareil edu de QEMU)"
CMDLINE = "selftest exit pci.q35 pci.edu"
QEMU_EXTRA = (
    "-device",
    "pci-bridge,id=pont1,chassis_nr=1",
    "-device",
    "edu,bus=pont1,addr=1",
)


def run(vm):
    vm.attendre(r"pci: \d+ appareil\(s\), configuration par ECAM", delai=60)
    vm.attendre(r"\[TEST\] pci \.\.\. OK", delai=120)
    texte = vm.serie()
    assert re.search(r"pci: 0000:00:03\.0 1b36:0001 060400 PCI-PCI bridge", texte), "pont absent"
    assert re.search(r"pci: 0000:01:01\.0 1234:11e8 ", texte), "edu derriere le pont absent"
