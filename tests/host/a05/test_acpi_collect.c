#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static void	collect_xsdt(void)
{
	t_acpi_src	s;
	t_acpi_root	root;
	t_acpi_set	set;

	facpi_reset();
	s = facpi_src();
	facpi_std(8);
	root.phys = F_XSDT;
	root.entry_size = 8;
	h_eq_i64("collecte xsdt", acpi_collect(&s, &root, &set), E_OK);
	h_eq_u64("tables gardees", set.count, 7);
	h_eq_u64("tables rejetees", set.rejected, 2);
	h_eq_u64("mappages = tables", g_fmem.maps - g_fmem.unmaps, set.count);
	h_true(acpi_set_find(&set, "SSDT", 1) != NULL, "deuxieme ssdt");
	h_true(!acpi_set_find(&set, "SSDT", 2), "troisieme ssdt absente");
	h_true(!acpi_set_find(&set, "ZZZZ", 0), "signature inconnue");
	h_true(!acpi_set_find(&set, NULL, 0), "signature nulle");
	h_true(acpi_set_find(&set, "DSDT", 0) != NULL, "dsdt par la fadt");
	h_true(acpi_set_find(&set, "XSDT", 0) != NULL, "racine trouvable");
}

static void	collect_rsdt(void)
{
	t_acpi_src	s;
	t_acpi_root	root;
	t_acpi_set	set;

	facpi_reset();
	s = facpi_src();
	facpi_std(8);
	root.phys = F_XSDT;
	root.entry_size = 4;
	h_eq_i64("racine de mauvaise signature", acpi_collect(&s, &root, &set),
		E_INVAL);
	facpi_std(4);
	root.phys = F_RSDT;
	h_eq_i64("collecte rsdt", acpi_collect(&s, &root, &set), E_OK);
	h_eq_u64("tables rsdt", set.count, 7);
	h_eq_u64("mappages = tables", g_fmem.maps - g_fmem.unmaps, set.count);
	facpi_at(F_DSDT)[0] = 'S';
	facpi_fix(F_DSDT);
	h_eq_i64("dsdt de mauvaise signature", acpi_collect(&s, &root, &set),
		E_OK);
	h_eq_u64("dsdt rejetee", set.rejected, 3);
}

int	main(void)
{
	h_begin("a05/acpi_collect");
	h_run("collecte xsdt", collect_xsdt);
	h_run("collecte rsdt et racine fausse", collect_rsdt);
	return (h_end());
}
