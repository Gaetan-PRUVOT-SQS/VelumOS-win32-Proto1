#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static const uint8_t	g_zero[64];

static void	sdt_limits(void)
{
	t_acpi_src	s;
	int			err;

	facpi_reset();
	s = facpi_src();
	facpi_hdr(F_APIC, "APIC", 36, 1);
	facpi_fix(F_APIC);
	h_true(acpi_map_table(&s, F_APIC, &err) != NULL, "longueur 36 acceptee");
	h_eq_u64("une table gardee", g_fmem.maps - g_fmem.unmaps, 1);
	facpi_hdr(F_APIC, "APIC", 0, 1);
	h_true(!acpi_map_table(&s, F_APIC, &err) && err == E_INVAL, "long. 0");
	facpi_hdr(F_APIC, "APIC", 35, 1);
	h_true(!acpi_map_table(&s, F_APIC, &err) && err == E_INVAL, "long. 35");
	facpi_hdr(F_APIC, "APIC", ACPI_TABLE_MAX_LEN + 1, 1);
	h_true(!acpi_map_table(&s, F_APIC, &err) && err == E_INVAL, "long. max");
	h_true(!acpi_map_table(&s, 0, &err) && err == E_INVAL, "adresse nulle");
	h_true(!acpi_map_table(&s, UINT64_MAX - 8, &err) && err == E_INVAL,
		"debordement d'adresse");
	h_eq_u64("mappages rendus", g_fmem.maps - g_fmem.unmaps, 1);
}

static void	sdt_bad_body(void)
{
	t_acpi_src	s;
	int			err;

	facpi_reset();
	s = facpi_src();
	facpi_hdr(FAKE_BASE + FAKE_SIZE - 64, "APIC", 100, 1);
	h_true(!acpi_map_table(&s, FAKE_BASE + FAKE_SIZE - 64, &err)
		&& err == E_FAULT, "table qui sort de la memoire");
	facpi_hdr(F_APIC, "APIC", 44, 1);
	facpi_fix(F_APIC);
	facpi_at(F_APIC)[40] ^= 0x40;
	h_true(!acpi_map_table(&s, F_APIC, &err) && err == E_INVAL, "somme");
	h_eq_u64("aucun mappage perdu", g_fmem.maps, g_fmem.unmaps);
}

static void	sdt_overflow(void)
{
	t_acpi_src	s;
	t_acpi_root	root;
	t_acpi_set	set;
	uint64_t	tabs[70];
	uint32_t	k;

	facpi_reset();
	s = facpi_src();
	k = 0;
	while (k < 70)
	{
		tabs[k] = 0xe9000 + k * 64;
		facpi_aml(tabs[k], "OEMX", g_zero, 0);
		k++;
	}
	facpi_root(F_XSDT, 8, tabs, 70);
	root.phys = F_XSDT;
	root.entry_size = 8;
	h_eq_i64("collecte pleine", acpi_collect(&s, &root, &set), E_OK);
	h_eq_u64("plafond 64 tables", set.count, ACPI_TABLES_MAX);
	h_eq_u64("tables en trop", set.dropped, 71 - ACPI_TABLES_MAX);
	h_eq_u64("mappages = tables", g_fmem.maps - g_fmem.unmaps, set.count);
}

int	main(void)
{
	h_begin("a05/acpi_sdt");
	h_run("limites d'une table", sdt_limits);
	h_run("table hors memoire ou somme fausse", sdt_bad_body);
	h_run("plus de 64 tables", sdt_overflow);
	return (h_end());
}
