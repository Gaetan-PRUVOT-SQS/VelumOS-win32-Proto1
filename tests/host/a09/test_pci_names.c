#include "a09_test.h"
#include "harness.h"
#include "pci_int.h"

static void	noms_exacts_et_de_repli(void)
{
	h_eq_str("06:00", pci_class_name(6, 0), "host bridge");
	h_eq_str("06:04", pci_class_name(6, 4), "PCI-PCI bridge");
	h_eq_str("03:00", pci_class_name(3, 0), "VGA compatible controller");
	h_eq_str("01:06", pci_class_name(1, 6), "SATA controller");
	h_eq_str("0c:03", pci_class_name(0x0c, 3), "USB controller");
	h_eq_str("02:00", pci_class_name(2, 0), "Ethernet controller");
	h_eq_str("sous-classe inconnue : nom de la classe",
		pci_class_name(1, 0x99), "storage");
	h_eq_str("classe sans table : repli", pci_class_name(0x7f, 0), "other");
	h_eq_str("classe 0xff", pci_class_name(0xff, 0xff), "unassigned class");
}

static void	types_de_bar_et_backends(void)
{
	h_eq_str("E/S", pci_bar_kind(PCI_BAR_IO), "io");
	h_eq_str("64 bits", pci_bar_kind(PCI_BAR_MEM64), "mem64");
	h_eq_str("64 bits prefetch", pci_bar_kind(PCI_BAR_MEM64 | PCI_BAR_PREFETCH),
		"mem64");
	h_eq_str("32 bits", pci_bar_kind(0), "mem32");
	h_eq_str("prefetch", pci_bar_pref(PCI_BAR_PREFETCH), " prefetch");
	h_eq_str("sans prefetch", pci_bar_pref(0), "");
	h_eq_str("ECAM", pci_backend_name(PCI_BACKEND_ECAM), "ECAM (MCFG)");
	h_eq_str("ports", pci_backend_name(PCI_BACKEND_LEGACY),
		"ports 0xcf8/0xcfc");
}

int	main(void)
{
	h_begin("a09/pci_names");
	h_run("names/classes-exactes-et-repli", noms_exacts_et_de_repli);
	h_run("names/bar-et-backend", types_de_bar_et_backends);
	return (h_end());
}
