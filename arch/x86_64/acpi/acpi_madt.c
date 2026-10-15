#include "acpi_int.h"
#include "velum/err.h"

static void	madt_lapic(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	if (len < 8)
	{
		x->madt_skipped++;
		return ;
	}
	if (acpi_rd(e, 4, 4) & MADT_LAPIC_ENABLED)
		madt_add_cpu(e[3], e[2], i, x);
}

static void	madt_x2apic(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	if (len < 16)
	{
		x->madt_skipped++;
		return ;
	}
	if (acpi_rd(e, 8, 4) & MADT_LAPIC_ENABLED)
		madt_add_cpu((uint32_t)acpi_rd(e, 4, 4), (uint32_t)acpi_rd(e, 12, 4),
			i, x);
}

static void	madt_ioapic(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	t_ioapic_info	*io;

	if (len < 12 || i->nioapics >= ACPI_MAX_IOAPICS)
	{
		x->madt_skipped++;
		return ;
	}
	io = &i->ioapic[i->nioapics];
	io->id = e[2];
	io->phys = acpi_rd(e, 4, 4);
	io->gsi_base = (uint32_t)acpi_rd(e, 8, 4);
	i->nioapics++;
}

void	madt_entry(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	if (e[0] == 0)
		madt_lapic(e, len, i, x);
	else if (e[0] == 1)
		madt_ioapic(e, len, i, x);
	else if (e[0] == 2)
		madt_iso(e, len, i, x);
	else if (e[0] == 3)
		madt_nmi_src(e, len, i, x);
	else if (e[0] == 4 || e[0] == 0xa)
		madt_nmi(e, len, i, x);
	else if (e[0] == 5)
		madt_lapic_ovr(e, len, i, x);
	else if (e[0] == 9)
		madt_x2apic(e, len, i, x);
}

int	acpi_parse_madt(const struct s_acpi_sdt *t, t_acpi_info *i,
	t_acpi_extra *x)
{
	const uint8_t	*p;
	uint32_t		off;
	uint8_t			elen;

	if (t->length < MADT_ENTRIES)
		return (E_INVAL);
	p = (const uint8_t *)t;
	i->lapic_phys = acpi_rd(p, MADT_LAPIC_ADDR, 4);
	x->madt_flags = (uint32_t)acpi_rd(p, MADT_FLAGS, 4);
	x->have_madt = true;
	off = MADT_ENTRIES;
	while (off + 2 <= t->length)
	{
		elen = p[off + 1];
		if (elen < 2 || off + elen > t->length)
		{
			x->madt_skipped++;
			break ;
		}
		madt_entry(p + off, elen, i, x);
		off += elen;
	}
	if (x->lapic_ovr)
		i->lapic_phys = x->lapic_ovr;
	return (E_OK);
}
