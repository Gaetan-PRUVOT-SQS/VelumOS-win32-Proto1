#include "pci_int.h"

static const t_pciclass	g_names[] = {
{0x00, 0xff, "unclassified"},
{0x01, 0xff, "storage"},
{0x01, 0x00, "SCSI storage"},
{0x01, 0x01, "IDE controller"},
{0x01, 0x02, "floppy controller"},
{0x01, 0x04, "RAID controller"},
{0x01, 0x05, "ATA controller"},
{0x01, 0x06, "SATA controller"},
{0x01, 0x07, "SAS controller"},
{0x01, 0x08, "NVM controller"},
{0x02, 0xff, "network"},
{0x02, 0x00, "Ethernet controller"},
{0x03, 0xff, "display"},
{0x03, 0x00, "VGA compatible controller"},
{0x03, 0x02, "3D controller"},
{0x04, 0xff, "multimedia"},
{0x04, 0x01, "audio device"},
{0x04, 0x03, "HD audio device"},
{0x05, 0xff, "memory controller"},
{0x06, 0xff, "bridge"},
{0x06, 0x00, "host bridge"},
{0x06, 0x01, "ISA bridge"},
{0x06, 0x04, "PCI-PCI bridge"},
{0x06, 0x07, "CardBus bridge"},
{0x07, 0xff, "communication"},
{0x07, 0x00, "serial controller"},
{0x08, 0xff, "system peripheral"},
{0x08, 0x05, "SD host controller"},
{0x09, 0xff, "input device"},
{0x0b, 0xff, "processor"},
{0x0c, 0xff, "serial bus"},
{0x0c, 0x03, "USB controller"},
{0x0c, 0x05, "SMBus controller"},
{0x0d, 0xff, "wireless"},
{0x10, 0xff, "encryption"},
{0x11, 0xff, "signal processing"},
{0xff, 0xff, "unassigned class"}
};

const char	*pci_class_name(uint8_t cls, uint8_t sub)
{
	const char	*generic;
	uint32_t	i;

	generic = "other";
	i = 0;
	while (i < sizeof(g_names) / sizeof(g_names[0]))
	{
		if (g_names[i].cls == cls && g_names[i].sub == sub)
			return (g_names[i].name);
		if (g_names[i].cls == cls && g_names[i].sub == 0xff)
			generic = g_names[i].name;
		i++;
	}
	return (generic);
}

const char	*pci_bar_kind(uint8_t flags)
{
	if (flags & PCI_BAR_IO)
		return ("io");
	if (flags & PCI_BAR_MEM64)
		return ("mem64");
	return ("mem32");
}

const char	*pci_bar_pref(uint8_t flags)
{
	if (flags & PCI_BAR_PREFETCH)
		return (" prefetch");
	return ("");
}

const char	*pci_backend_name(int backend)
{
	if (backend == PCI_BACKEND_ECAM)
		return ("ECAM (MCFG)");
	return ("ports 0xcf8/0xcfc");
}
