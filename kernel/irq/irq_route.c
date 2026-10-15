#include "irq_int.h"

static const t_irq_override	*override_for_gsi(const t_acpi_info *ai,
	uint32_t gsi)
{
	uint32_t	k;

	k = 0;
	while (k < ai->noverrides && k < ACPI_MAX_OVERRIDES)
	{
		if (ai->override[k].gsi == gsi)
			return (&ai->override[k]);
		k++;
	}
	return (NULL);
}

static uint32_t	trig_from_mps(uint16_t flags)
{
	uint32_t	trig;

	trig = 0;
	if ((flags & ACPI_POL_MASK) == ACPI_POL_LOW)
		trig |= IRQF_LOW;
	if ((flags & ACPI_TRIG_MASK) == ACPI_TRIG_LEVEL)
		trig |= IRQF_LEVEL;
	return (trig);
}

uint32_t	irq_resolve_trig(const t_acpi_info *ai, uint32_t gsi,
	uint32_t flags)
{
	const t_irq_override	*o;

	o = override_for_gsi(ai, gsi);
	if (o)
		return (trig_from_mps(o->flags));
	if (flags & IRQF_TRIG_MASK)
		return (flags & IRQF_TRIG_MASK);
	if (gsi < IRQ_ISA_LINES)
		return (0);
	return (IRQF_LEVEL | IRQF_LOW);
}

uint32_t	irq_isa_lookup(const t_acpi_info *ai, uint8_t isa)
{
	uint32_t	k;

	k = 0;
	while (k < ai->noverrides && k < ACPI_MAX_OVERRIDES)
	{
		if (ai->override[k].isa == isa)
			return (ai->override[k].gsi);
		k++;
	}
	return (isa);
}
