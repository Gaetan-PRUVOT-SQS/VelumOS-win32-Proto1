#include "a09_test.h"
#include "harness.h"
#include "pci_int.h"

static void	encodage_des_champs_cf8(void)
{
	t_pciloc	l;

	l = (t_pciloc){0, 0, 0, 0};
	h_eq_u64("00:00.0 reg 0", pci_legacy_addr(&l, 0), 0x80000000);
	l = (t_pciloc){0, 1, 2, 3};
	h_eq_u64("01:02.3 reg 0x10", pci_legacy_addr(&l, 0x10), 0x80011310);
	l = (t_pciloc){0, 255, 31, 7};
	h_eq_u64("ff:1f.7 reg 0xfc", pci_legacy_addr(&l, 0xfc), 0x80fffffc);
	h_eq_u64("bits 1:0 du registre ignores (0x3e)",
		pci_legacy_addr(&l, 0x3e), 0x80ffff3c);
	h_eq_u64("octet 255 : dernier registre", pci_legacy_addr(&l, 255),
		0x80fffffc);
}

static void	refus_hors_limites(void)
{
	t_pciloc	l;

	l = (t_pciloc){0, 0, 0, 0};
	h_eq_u64("offset 256", pci_legacy_addr(&l, 256), 0);
	h_eq_u64("offset 4095", pci_legacy_addr(&l, 4095), 0);
	l = (t_pciloc){0, 0, 32, 0};
	h_eq_u64("appareil 32", pci_legacy_addr(&l, 0), 0);
	l = (t_pciloc){0, 0, 0, 8};
	h_eq_u64("fonction 8", pci_legacy_addr(&l, 0), 0);
	l = (t_pciloc){1, 0, 0, 0};
	h_eq_u64("segment 1 : pas de ports", pci_legacy_addr(&l, 0), 0);
}

int	main(void)
{
	h_begin("a09/pci_legacy_addr");
	h_run("legacy/encodage-cf8-bus-appareil-fonction-registre",
		encodage_des_champs_cf8);
	h_run("legacy/refus-offset-256-appareil-32-fonction-8-segment",
		refus_hors_limites);
	return (h_end());
}
