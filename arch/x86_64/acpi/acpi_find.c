#include "acpi_int.h"
#include "velum/klog.h"

const t_acpi_info	*acpi_info(void)
{
	return (&acpi_state()->info);
}

const t_acpi_extra	*acpi_extra(void)
{
	return (&acpi_state()->x);
}

const struct s_acpi_sdt	*acpi_find_table(const char *sig, uint32_t index)
{
	return (acpi_set_find(&acpi_state()->set, sig, index));
}

static void	acpi_log_tables(const t_acpi_set *set)
{
	const struct s_acpi_sdt	*t;
	uint32_t				k;

	k = 0;
	while (k < set->count)
	{
		t = set->tab[k].hdr;
		klog_info("acpi: %.4s @%#llx long. %u rév. %u oem %.6s", t->sig,
			set->tab[k].phys, t->length, t->revision, t->oem_id);
		k++;
	}
}

void	acpi_log(const t_acpi_state *st)
{
	const t_acpi_info	*i;
	const char			*off;
	const char			*root;

	i = &st->info;
	root = "RSDT";
	if (st->root.entry_size == 8)
		root = "XSDT";
	klog_info("acpi: racine %s @%#llx, %u tables, %u rejetées, %u ignorées",
		root, st->root.phys, st->set.count, st->set.rejected,
		st->set.dropped);
	acpi_log_tables(&st->set);
	klog_info("acpi: %u CPU, LAPIC @%#llx, %u IOAPIC, %u redirections, "
		"%u entrées MADT ignorées", i->ncpus, i->lapic_phys, i->nioapics,
		i->noverrides, st->x.madt_skipped);
	off = "non supportée";
	if (i->poweroff_ok)
		off = "ACPI S5";
	klog_info("acpi: HPET @%#llx, %u MCFG, SCI %u, siècle CMOS %#x, "
		"extinction %s (SLP_TYP %u/%u)", i->hpet_phys, i->nmcfg, i->sci_irq,
		i->century_reg, off, i->slp_typ_a, i->slp_typ_b);
}
