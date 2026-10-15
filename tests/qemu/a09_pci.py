import re

NAME = "a09 : PCI par ECAM sous q35 (hote, VGA, ponts), autotest pci"
CMDLINE = "selftest exit pci.q35"


def run(vm):
    vm.attendre(r"pci: \d+ appareil\(s\), configuration par ECAM \(MCFG\)", delai=60)
    vm.attendre(r"boot: pci ok", delai=60)
    vm.attendre(r"\[TEST\] pci \.\.\. OK", delai=120)
    texte = vm.serie()
    assert re.search(r"pci: 0000:00:00\.0 8086:29c0 060000 host bridge", texte), "hote q35 absent"
    assert re.search(r"pci: 0000:00:01\.0 1234:1111 030000 VGA compatible", texte), "VGA absente"
    assert re.search(r"pci: 0000:00:1f\.0 8086:2918 0601\w\w ISA bridge", texte), "pont ISA absent"
    assert "pci: controle" not in texte, "autotest pci en echec"
