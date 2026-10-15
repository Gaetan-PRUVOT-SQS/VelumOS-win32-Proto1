#include <string.h>
#include "fake.h"

static void	fadt_gas(uint32_t off, uint8_t space, uint8_t width, uint64_t addr)
{
	uint8_t	*p;

	p = facpi_at(F_FACP) + off;
	p[0] = space;
	p[1] = width;
	p[2] = 0;
	p[3] = 1;
	facpi_put(F_FACP, off + GAS_ADDR, addr, 8);
}

void	facpi_fadt(uint32_t len, uint64_t dsdt)
{
	memset(facpi_at(F_FACP), 0, 0x200);
	facpi_hdr(F_FACP, "FACP", len, 6);
	facpi_put(F_FACP, FADT_DSDT, dsdt, 4);
	facpi_put(F_FACP, FADT_PROFILE, 1, 1);
	facpi_put(F_FACP, FADT_SCI_INT, 9, 2);
	facpi_put(F_FACP, FADT_SMI_CMD, 0xb2, 4);
	facpi_put(F_FACP, FADT_ACPI_ENABLE, 0xf1, 1);
	facpi_put(F_FACP, FADT_PM1A_CNT, 0x604, 4);
	facpi_put(F_FACP, FADT_PM1_CNT_LEN, 2, 1);
	facpi_put(F_FACP, FADT_CENTURY, 0x32, 1);
	facpi_put(F_FACP, FADT_FLAGS, FADT_RESET_SUP, 4);
	fadt_gas(FADT_RESET_REG, GAS_SPACE_IO, 8, 0xcf9);
	facpi_put(F_FACP, FADT_RESET_VALUE, 0x06, 1);
	facpi_put(F_FACP, FADT_X_DSDT, dsdt, 8);
	fadt_gas(FADT_X_PM1A_CNT, GAS_SPACE_IO, 16, 0x604);
	facpi_fix(F_FACP);
}

void	facpi_aml(uint64_t phys, const char *sig, const uint8_t *aml,
	uint32_t n)
{
	facpi_hdr(phys, sig, ACPI_HDR_LEN + n, 2);
	memcpy(facpi_at(phys) + ACPI_HDR_LEN, aml, n);
	facpi_fix(phys);
}
