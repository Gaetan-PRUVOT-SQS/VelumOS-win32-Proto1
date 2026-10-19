#include "harness.h"
#include "apic_int.h"

static void	icr_values(void)
{
	h_eq_u64("INIT assert niveau, SDM 10.6.1", lapic_icr_value(LAPIC_ICR_INIT, 0),
		0xc500);
	h_eq_u64("SIPI page 0x08", lapic_icr_value(LAPIC_ICR_SIPI, 0x08), 0x4608);
	h_eq_u64("SIPI page 0x9f", lapic_icr_value(LAPIC_ICR_SIPI, 0x9f), 0x469f);
	h_eq_u64("fixe 0xf1", lapic_icr_value(LAPIC_ICR_FIXED, 0xf1), 0x40f1);
	h_eq_u64("fixe 0x20", lapic_icr_value(LAPIC_ICR_FIXED, 0x20), 0x4020);
}

static void	icr_x2(void)
{
	h_eq_u64("x2apic destination en haut, SDM 10.12.9",
		lapic_icr_x2(0x40f1, 5), 0x00000005000040f1ull);
	h_eq_u64("x2apic grand identifiant", lapic_icr_x2(0xc500, 0x12345678),
		0x123456780000c500ull);
}

int	main(void)
{
	h_begin("e07/lapic_icr");
	h_run("valeurs de l'ICR", icr_values);
	h_run("ICR x2apic", icr_x2);
	return (h_end());
}
