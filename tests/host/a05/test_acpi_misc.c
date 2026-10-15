#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static void	fadt_no_reset_sup(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_fadt(276, F_DSDT);
	facpi_put(F_FACP, FADT_FLAGS, 0, 4);
	facpi_fix(F_FACP);
	memset(&i, 0, sizeof(i));
	memset(&x, 0, sizeof(x));
	acpi_parse_fadt((const struct s_acpi_sdt *)facpi_at(F_FACP), &i, &x);
	h_eq_u64("reset sans drapeau refuse", i.reset_port, 0);
	h_eq_u64("pm1a garde", i.pm1a_cnt, 0x604);
}

static void	madt_ioapic_cap(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;
	uint8_t			e[9 * 12];
	uint32_t		k;

	k = 0;
	while (k < 9)
	{
		memcpy(e + k * 12, "\x01\x0c\0\0\0\0\xc0\xfe\0\0\0\0", 12);
		e[k * 12 + 2] = (uint8_t)k;
		k++;
	}
	facpi_reset();
	facpi_madt(e, sizeof(e));
	memset(&i, 0, sizeof(i));
	memset(&x, 0, sizeof(x));
	h_eq_i64("madt ioapic rc", acpi_parse_madt(
			(const struct s_acpi_sdt *)facpi_at(F_APIC), &i, &x), E_OK);
	h_eq_u64("plafond ioapic", i.nioapics, ACPI_MAX_IOAPICS);
	h_eq_u64("ioapic en trop", x.madt_skipped, 9 - ACPI_MAX_IOAPICS);
	h_eq_u64("dernier ioapic garde", i.ioapic[7].id, 7);
}

int	main(void)
{
	h_begin("a05/acpi_divers");
	h_run("madt plafond ioapic", madt_ioapic_cap);
	h_run("fadt sans reset_reg_sup", fadt_no_reset_sup);
	return (h_end());
}
