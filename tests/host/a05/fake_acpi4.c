#include <string.h>
#include "fake.h"

static const uint8_t	g_zero[64];

void	facpi_std(uint32_t esize)
{
	uint64_t	tabs[8];

	tabs[0] = F_FACP;
	tabs[1] = F_APIC;
	tabs[2] = F_HPET;
	tabs[3] = F_SSDT;
	tabs[4] = F_SSDT2;
	tabs[5] = 0xfff00000;
	tabs[6] = F_MCFG;
	tabs[7] = F_APIC;
	facpi_fadt(276, F_DSDT);
	facpi_aml(F_APIC, "APIC", g_zero, 8);
	facpi_aml(F_HPET, "HPET", g_zero, 20);
	facpi_aml(F_SSDT, "SSDT", g_zero, 1);
	facpi_aml(F_SSDT2, "SSDT", g_zero, 1);
	facpi_aml(F_DSDT, "DSDT", g_zero, 1);
	facpi_aml(F_MCFG, "MCFG", g_zero, 1);
	facpi_at(F_MCFG)[9] ^= 1;
	if (esize == 8)
		facpi_root(F_XSDT, 8, tabs, 8);
	else
		facpi_root(F_RSDT, 4, tabs, 8);
}

void	facpi_madt(const uint8_t *entries, uint32_t n)
{
	facpi_hdr(F_APIC, "APIC", MADT_ENTRIES + n, 5);
	facpi_put(F_APIC, MADT_LAPIC_ADDR, 0xfee00000, 4);
	facpi_put(F_APIC, MADT_FLAGS, MADT_PCAT_COMPAT, 4);
	memcpy(facpi_at(F_APIC) + MADT_ENTRIES, entries, n);
	facpi_fix(F_APIC);
}
