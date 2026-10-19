NAME = "e19 : virtio-net, DHCP QEMU user-net et écho ICMP vers la passerelle"
CMDLINE = "net exit"
FIRMWARES = ("bios",)
QEMU_EXTRA = [
    "-netdev",
    "user,id=n0",
    "-device",
    "virtio-net-pci,netdev=n0,disable-legacy=on",
    "-object",
    "filter-dump,id=f0,netdev=n0,file=build/T19/trafic.pcap",
]


def run(vm):
    vm.attendre(r"net: virtio-net ([0-9a-f]{2}(:[0-9a-f]{2}){5})", delai=60)
    bail = vm.attendre(
        r"net: bail (\d+\.\d+\.\d+\.\d+) masque (\d+\.\d+\.\d+\.\d+) "
        r"passerelle (\d+\.\d+\.\d+\.\d+) dns (\d+\.\d+\.\d+\.\d+)",
        delai=30,
    )
    assert bail.group(1) == "10.0.2.15", bail.group(0)
    assert bail.group(2) == "255.255.255.0", bail.group(0)
    assert bail.group(3) == "10.0.2.2", bail.group(0)
    assert bail.group(4) == "10.0.2.3", bail.group(0)
    vm.attendre(r"net: écho de la passerelle reçu en \d+ us", delai=30)
    assert "net: DHCP sans bail" not in vm.serie(), vm.serie()[-2000:]
