#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static int	parse(t_acpi_info *i, t_acpi_extra *x)
{
	memset(i, 0, sizeof(*i));
	memset(x, 0, sizeof(*x));
	facpi_fix(F_FACP);
	return (acpi_parse_fadt((const struct s_acpi_sdt *)facpi_at(F_FACP), i,
			x));
}

static void	fadt_versions(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_fadt(276, F_DSDT);
	h_eq_i64("fadt v6 rc", parse(&i, &x), E_OK);
	h_eq_u64("pm1a par x_pm1a", i.pm1a_cnt, 0x604);
	h_eq_u64("pm1b absent", i.pm1b_cnt, 0);
	h_eq_u64("sci", i.sci_irq, 9);
	h_eq_u64("siecle", i.century_reg, 0x32);
	h_eq_u64("reset port", i.reset_port, 0xcf9);
	h_eq_u64("reset valeur", i.reset_value, 6);
	h_eq_u64("smi_cmd", x.smi_cmd, 0xb2);
	h_eq_u64("acpi_enable", x.acpi_enable, 0xf1);
	h_eq_u64("dsdt 64 bits", acpi_fadt_dsdt(
			(const struct s_acpi_sdt *)facpi_at(F_FACP)), F_DSDT);
	facpi_fadt(116, F_DSDT);
	h_eq_i64("fadt v1 rc", parse(&i, &x), E_OK);
	h_eq_u64("v1 pm1a 32 bits", i.pm1a_cnt, 0x604);
	h_eq_u64("v1 sans reset", i.reset_port, 0);
	h_eq_u64("v1 siecle", i.century_reg, 0x32);
	facpi_fadt(115, F_DSDT);
	h_eq_i64("fadt 115 octets", parse(&i, &x), E_INVAL);
	h_true(!x.have_fadt, "fadt courte ignoree");
}

static void	fadt_pm1(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_fadt(276, F_DSDT);
	facpi_put(F_FACP, FADT_PM1A_CNT, 0x404, 4);
	facpi_put(F_FACP, FADT_X_PM1A_CNT, GAS_SPACE_MEM, 1);
	parse(&i, &x);
	h_eq_u64("x_pm1a en memoire repli 32 bits", i.pm1a_cnt, 0x404);
	facpi_put(F_FACP, FADT_X_PM1A_CNT, GAS_SPACE_IO, 1);
	facpi_put(F_FACP, FADT_X_PM1A_CNT + GAS_ADDR, 0x10000, 8);
	parse(&i, &x);
	h_eq_u64("x_pm1a hors ports repli", i.pm1a_cnt, 0x404);
	facpi_put(F_FACP, FADT_PM1A_CNT, 0x10000, 4);
	parse(&i, &x);
	h_eq_u64("pm1a hors ports refuse", i.pm1a_cnt, 0);
	facpi_fadt(276, F_DSDT);
	facpi_put(F_FACP, FADT_PM1B_CNT, 0x608, 4);
	parse(&i, &x);
	h_eq_u64("pm1b 32 bits", i.pm1b_cnt, 0x608);
	facpi_put(F_FACP, FADT_FLAGS, FADT_HW_REDUCED, 4);
	parse(&i, &x);
	h_true(!i.pm1a_cnt && !i.pm1b_cnt, "hw reduced sans pm1");
	h_eq_u64("hw reduced sans reset", i.reset_port, 0);
}

static void	fadt_reset_century(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_fadt(276, F_DSDT);
	facpi_put(F_FACP, FADT_RESET_REG + 1, 16, 1);
	parse(&i, &x);
	h_eq_u64("reset largeur 16 refuse", i.reset_port, 0);
	facpi_fadt(276, F_DSDT);
	facpi_put(F_FACP, FADT_RESET_REG, GAS_SPACE_MEM, 1);
	parse(&i, &x);
	h_eq_u64("reset en memoire refuse", i.reset_port, 0);
	facpi_fadt(276, F_DSDT);
	facpi_put(F_FACP, FADT_CENTURY, 0x0d, 1);
	parse(&i, &x);
	h_eq_u64("siecle 0x0d refuse", i.century_reg, 0);
	facpi_put(F_FACP, FADT_CENTURY, 0x0e, 1);
	parse(&i, &x);
	h_eq_u64("siecle 0x0e garde", i.century_reg, 0x0e);
	facpi_put(F_FACP, FADT_CENTURY, 0x80, 1);
	parse(&i, &x);
	h_eq_u64("siecle 0x80 refuse", i.century_reg, 0);
	facpi_put(F_FACP, FADT_X_DSDT, 0, 8);
	h_eq_u64("dsdt 32 bits si x_dsdt nul", acpi_fadt_dsdt(
			(const struct s_acpi_sdt *)facpi_at(F_FACP)), F_DSDT);
}

int	main(void)
{
	h_begin("a05/acpi_fadt");
	h_run("fadt versions 1 et 6", fadt_versions);
	h_run("fadt registres pm1", fadt_pm1);
	h_run("fadt reset et siecle", fadt_reset_century);
	return (h_end());
}
