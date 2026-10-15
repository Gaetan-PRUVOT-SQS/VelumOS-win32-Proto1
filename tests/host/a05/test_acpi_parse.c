#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static const uint8_t	g_s5[] = {0x10, 0x08, 0x08, '_', 'S', '5', '_', 0x12,
	0x06, 0x04, 0x0a, 0x05, 0x0a, 0x05};
static const uint8_t	g_none[] = {0x10, 0x08, 0x08, '_', 'S', '4', '_', 0x12,
	0x06, 0x04, 0x0a, 0x05, 0x0a, 0x05};

static void	collect_parse(t_acpi_set *set, t_acpi_info *i, t_acpi_extra *x)
{
	t_acpi_src	s;
	t_acpi_root	root;

	s = facpi_src();
	root.phys = F_XSDT;
	root.entry_size = 8;
	memset(i, 0, sizeof(*i));
	memset(x, 0, sizeof(*x));
	acpi_collect(&s, &root, set);
	acpi_parse_all(set, i, x);
}

static void	parse_poweroff(void)
{
	t_acpi_set		set;
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_std(8);
	facpi_aml(F_DSDT, "DSDT", g_s5, sizeof(g_s5));
	collect_parse(&set, &i, &x);
	h_true(i.poweroff_ok && i.slp_typ_a == 5 && i.slp_typ_b == 5,
		"s5 dans le dsdt");
	facpi_aml(F_DSDT, "DSDT", g_none, sizeof(g_none));
	facpi_aml(F_SSDT2, "SSDT", g_s5, sizeof(g_s5));
	collect_parse(&set, &i, &x);
	h_true(i.poweroff_ok && x.have_s5, "s5 dans une ssdt");
	facpi_aml(F_SSDT2, "SSDT", g_none, sizeof(g_none));
	collect_parse(&set, &i, &x);
	h_true(!i.poweroff_ok && !x.have_s5, "sans s5 pas d'extinction");
	facpi_aml(F_DSDT, "DSDT", g_s5, sizeof(g_s5));
	facpi_put(F_FACP, FADT_FLAGS, FADT_HW_REDUCED, 4);
	facpi_fix(F_FACP);
	collect_parse(&set, &i, &x);
	h_true(!i.poweroff_ok && x.have_s5, "hw reduced sans pm1");
}

static void	parse_hpet(void)
{
	const struct s_acpi_sdt	*t;
	t_acpi_info				i;

	facpi_reset();
	t = (const struct s_acpi_sdt *)facpi_at(F_HPET);
	facpi_hdr(F_HPET, "HPET", HPET_TABLE_MIN, 1);
	facpi_put(F_HPET, HPET_TABLE_ADDR + GAS_ADDR, 0xfed00000, 8);
	memset(&i, 0, sizeof(i));
	h_eq_i64("hpet valide", acpi_parse_hpet(t, &i), E_OK);
	h_eq_u64("hpet adresse", i.hpet_phys, 0xfed00000);
	facpi_put(F_HPET, HPET_TABLE_ADDR, GAS_SPACE_IO, 1);
	h_eq_i64("hpet en espace es", acpi_parse_hpet(t, &i), E_INVAL);
	facpi_put(F_HPET, HPET_TABLE_ADDR, GAS_SPACE_MEM, 1);
	facpi_put(F_HPET, HPET_TABLE_ADDR + GAS_ADDR, 0, 8);
	h_eq_i64("hpet adresse nulle", acpi_parse_hpet(t, &i), E_INVAL);
	facpi_put(F_HPET, 4, HPET_TABLE_MIN - 1, 4);
	h_eq_i64("hpet trop courte", acpi_parse_hpet(t, &i), E_INVAL);
}

static void	parse_mcfg(void)
{
	const struct s_acpi_sdt	*t;
	t_acpi_info				i;
	uint32_t				k;

	facpi_reset();
	t = (const struct s_acpi_sdt *)facpi_at(F_MCFG);
	facpi_hdr(F_MCFG, "MCFG", MCFG_ENTRIES + 10 * MCFG_ENTRY_LEN, 1);
	k = 0;
	while (k < 10)
	{
		facpi_put(F_MCFG, MCFG_ENTRIES + k * 16, 0xb0000000 + k, 8);
		facpi_put(F_MCFG, MCFG_ENTRIES + k * 16 + 11, 0xff, 1);
		k++;
	}
	facpi_put(F_MCFG, MCFG_ENTRIES + 16 + 10, 2, 1);
	facpi_put(F_MCFG, MCFG_ENTRIES + 16 + 11, 1, 1);
	memset(&i, 0, sizeof(i));
	h_eq_i64("mcfg rc", acpi_parse_mcfg(t, &i), E_OK);
	h_eq_u64("mcfg plafond et entree invalide", i.nmcfg, ACPI_MAX_MCFG);
	h_eq_u64("mcfg deuxieme gardee = troisieme", i.mcfg[1].base,
		0xb0000002);
	facpi_put(F_MCFG, 4, MCFG_ENTRIES - 1, 4);
	h_eq_i64("mcfg trop courte", acpi_parse_mcfg(t, &i), E_INVAL);
}

int	main(void)
{
	h_begin("a05/acpi_parse");
	h_run("extinction selon dsdt ssdt fadt", parse_poweroff);
	h_run("table hpet", parse_hpet);
	h_run("table mcfg", parse_mcfg);
	return (h_end());
}
