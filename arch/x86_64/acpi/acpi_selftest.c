#include "acpi_int.h"
#include "velum/klog.h"

static int	check_tables(const t_acpi_set *set)
{
	uint32_t				k;
	int						fails;
	const struct s_acpi_sdt	*t;

	k = 0;
	fails = 0;
	while (k < set->count)
	{
		t = set->tab[k].hdr;
		if (t->length < ACPI_HDR_LEN || acpi_sum(t, t->length) != 0)
		{
			klog_err("acpi: table %.4s altérée", t->sig);
			fails++;
		}
		k++;
	}
	return (fails);
}

static int	check_lookup(const t_acpi_state *st)
{
	int	fails;

	fails = 0;
	if (acpi_find_table(NULL, 0) || acpi_find_table("ZZZZ", 0))
		fails++;
	if ((acpi_find_table("APIC", 0) != NULL) != st->x.have_madt)
		fails++;
	if (acpi_find_table("APIC", ACPI_TABLES_MAX))
		fails++;
	if (st->x.have_madt && st->info.ncpus == 0)
		fails++;
	if (st->info.poweroff_ok && !st->info.pm1a_cnt)
		fails++;
	return (fails);
}

int	acpi_selftest(void)
{
	const t_acpi_state	*st;
	int					fails;

	st = acpi_state();
	if (st->set.count == 0)
	{
		klog_warn("acpi: aucune table, autotest réduit");
		return (0);
	}
	fails = check_tables(&st->set) + check_lookup(st);
	if (fails)
		klog_err("acpi: autotest, %d échec(s)", fails);
	return (fails);
}
