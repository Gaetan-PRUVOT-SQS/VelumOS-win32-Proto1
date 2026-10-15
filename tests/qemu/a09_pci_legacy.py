NAME = "a09 : PCI par les ports 0xcf8/0xcfc sous i440fx (aucune table MCFG)"
CMDLINE = "selftest exit"
QEMU_EXTRA = ("-machine", "pc")


def run(vm):
    vm.attendre(r"pci: \d+ appareil\(s\), configuration par ports 0xcf8/0xcfc", delai=60)
    vm.attendre(r"pci: 0000:00:00\.0 8086:1237 060000 host bridge", delai=60)
    vm.attendre(r"\[TEST\] pci \.\.\. OK", delai=120)
