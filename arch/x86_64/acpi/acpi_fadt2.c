#include "acpi_int.h"

static uint8_t	fadt_century(uint64_t reg)
{
	if (reg < CMOS_FIRST_FREE || reg > CMOS_LAST_INDEX)
		return (0);
	return ((uint8_t)reg);
}

void	fadt_basic(const struct s_acpi_sdt *t, t_acpi_info *i,
	t_acpi_extra *x)
{
	uint64_t	v;

	if (fadt_get(t, FADT_SCI_INT, 2, &v))
		i->sci_irq = (uint16_t)v;
	if (fadt_get(t, FADT_PROFILE, 1, &v))
		x->pm_profile = (uint8_t)v;
	if (fadt_get(t, FADT_SMI_CMD, 4, &v) && v <= 0xffff)
		x->smi_cmd = (uint16_t)v;
	if (fadt_get(t, FADT_ACPI_ENABLE, 1, &v))
		x->acpi_enable = (uint8_t)v;
	if (fadt_get(t, FADT_PM1_CNT_LEN, 1, &v))
		x->pm1_cnt_len = (uint8_t)v;
	if (fadt_get(t, FADT_CENTURY, 1, &v))
		i->century_reg = fadt_century(v);
	if (fadt_get(t, FADT_BOOT_ARCH, 2, &v))
		x->boot_arch = (uint16_t)v;
	if (fadt_get(t, FADT_FLAGS, 4, &v))
		x->fadt_flags = (uint32_t)v;
}

uint64_t	acpi_fadt_dsdt(const struct s_acpi_sdt *t)
{
	uint64_t	v;

	if (fadt_get(t, FADT_X_DSDT, 8, &v) && v)
		return (v);
	if (fadt_get(t, FADT_DSDT, 4, &v))
		return (v);
	return (0);
}
