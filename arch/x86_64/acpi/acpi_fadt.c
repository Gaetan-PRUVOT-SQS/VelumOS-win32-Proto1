#include "acpi_int.h"
#include "velum/err.h"

bool	fadt_get(const struct s_acpi_sdt *t, uint32_t off, uint32_t size,
	uint64_t *out)
{
	if ((uint64_t)off + size > t->length)
		return (false);
	*out = acpi_rd(t, off, size);
	return (true);
}

static uint16_t	fadt_gas_port(const struct s_acpi_sdt *t, uint32_t off)
{
	uint64_t	space;
	uint64_t	addr;

	if (!fadt_get(t, off, 1, &space) || !fadt_get(t, off + GAS_ADDR, 8, &addr))
		return (0);
	if (space != GAS_SPACE_IO || addr > 0xffff)
		return (0);
	return ((uint16_t)addr);
}

static void	fadt_pm1(const struct s_acpi_sdt *t, t_acpi_info *i)
{
	uint64_t	v;

	i->pm1a_cnt = fadt_gas_port(t, FADT_X_PM1A_CNT);
	if (!i->pm1a_cnt && fadt_get(t, FADT_PM1A_CNT, 4, &v) && v <= 0xffff)
		i->pm1a_cnt = (uint16_t)v;
	i->pm1b_cnt = fadt_gas_port(t, FADT_X_PM1B_CNT);
	if (!i->pm1b_cnt && fadt_get(t, FADT_PM1B_CNT, 4, &v) && v <= 0xffff)
		i->pm1b_cnt = (uint16_t)v;
}

static void	fadt_reset(const struct s_acpi_sdt *t, t_acpi_info *i,
	t_acpi_extra *x)
{
	uint64_t	width;
	uint64_t	offset;
	uint64_t	value;

	if (!(x->fadt_flags & FADT_RESET_SUP)
		|| !fadt_get(t, FADT_RESET_REG + 1, 1, &width)
		|| !fadt_get(t, FADT_RESET_REG + 2, 1, &offset)
		|| !fadt_get(t, FADT_RESET_VALUE, 1, &value)
		|| width != 8 || offset != 0)
		return ;
	i->reset_port = fadt_gas_port(t, FADT_RESET_REG);
	i->reset_value = (uint8_t)value;
}

int	acpi_parse_fadt(const struct s_acpi_sdt *t, t_acpi_info *i,
	t_acpi_extra *x)
{
	if (t->length < FADT_MIN_LEN)
		return (E_INVAL);
	x->have_fadt = true;
	fadt_basic(t, i, x);
	fadt_pm1(t, i);
	fadt_reset(t, i, x);
	if (x->fadt_flags & FADT_HW_REDUCED)
	{
		i->pm1a_cnt = 0;
		i->pm1b_cnt = 0;
	}
	return (E_OK);
}
