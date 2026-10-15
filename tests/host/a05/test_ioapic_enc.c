#include "harness.h"
#include "apic_int.h"
#include "velum/irq.h"

static void	route_ioapic(void)
{
	h_eq_u64("entree simple", ioapic_entry(0x31, 0, 0, false), 0x31);
	h_eq_u64("polarite basse bit 13", ioapic_entry(0x31, IRQF_LOW, 0, false),
		0x31 | 0x2000);
	h_eq_u64("niveau bit 15", ioapic_entry(0x31, IRQF_LEVEL, 0, false),
		0x31 | 0x8000);
	h_eq_u64("masque bit 16", ioapic_entry(0x31, 0, 0, true), 0x10031);
	h_eq_u64("destination bits 56-63", ioapic_entry(0x40, 0, 3, false),
		0x0300000000000040ull);
	h_eq_u64("destination tronquee", ioapic_entry(0x40, 0, 0x1ff, false),
		0xff00000000000040ull);
	h_eq_u64("ver 82093aa 24 entrees", ioapic_count_from_ver(0x00170011), 24);
	h_eq_u64("ver 0 une entree", ioapic_count_from_ver(0), 1);
	h_eq_u64("ver 255 plafond", ioapic_count_from_ver(0x00ff0020),
		IOAPIC_MAX_PINS);
	h_eq_u64("ver 239 juste sous plafond", ioapic_count_from_ver(0x00ef0020),
		240);
}

int	main(void)
{
	h_begin("a05/ioapic_enc");
	h_run("encodage ioapic", route_ioapic);
	return (h_end());
}
