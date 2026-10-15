#include "acpi_int.h"

void	madt_iso(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	t_irq_override	*o;

	if (len < 10 || e[2] != 0 || i->noverrides >= ACPI_MAX_OVERRIDES)
	{
		x->madt_skipped++;
		return ;
	}
	o = &i->override[i->noverrides];
	o->isa = e[3];
	o->reserved = 0;
	o->gsi = (uint32_t)acpi_rd(e, 4, 4);
	o->flags = (uint16_t)acpi_rd(e, 8, 2);
	i->noverrides++;
}

static void	nmi_fill(const uint8_t *e, t_acpi_nmi *n)
{
	n->reserved = 0;
	if (e[0] == 4)
	{
		n->uid = e[2];
		if (e[2] == 0xff)
			n->uid = ACPI_UID_ALL;
		n->flags = (uint16_t)acpi_rd(e, 3, 2);
		n->lint = e[5];
		return ;
	}
	n->uid = (uint32_t)acpi_rd(e, 4, 4);
	n->flags = (uint16_t)acpi_rd(e, 2, 2);
	n->lint = e[8];
}

void	madt_nmi(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	(void)i;
	if ((e[0] == 4 && len < 6) || (e[0] == 0xa && len < 12)
		|| x->nnmi >= ACPI_MAX_NMI)
	{
		x->madt_skipped++;
		return ;
	}
	nmi_fill(e, &x->nmi[x->nnmi]);
	if (x->nmi[x->nnmi].lint > 1)
		x->madt_skipped++;
	else
		x->nnmi++;
}

void	madt_nmi_src(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	(void)i;
	if (len < 8 || x->nnmi_src >= ACPI_MAX_NMI)
	{
		x->madt_skipped++;
		return ;
	}
	x->nmi_src_gsi[x->nnmi_src] = (uint32_t)acpi_rd(e, 4, 4);
	x->nnmi_src++;
}

void	madt_lapic_ovr(const uint8_t *e, uint8_t len, t_acpi_info *i,
	t_acpi_extra *x)
{
	(void)i;
	if (len < 12)
	{
		x->madt_skipped++;
		return ;
	}
	x->lapic_ovr = acpi_rd(e, 4, 8);
}
