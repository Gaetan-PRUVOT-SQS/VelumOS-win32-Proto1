#include "apic_int.h"
#include "velum/irq.h"

uint64_t	ioapic_entry(uint8_t vec, uint32_t trig, uint32_t dest,
	bool masked)
{
	uint64_t	e;

	e = vec;
	if (trig & IRQF_LOW)
		e |= IOAPIC_RED_LOW;
	if (trig & IRQF_LEVEL)
		e |= IOAPIC_RED_LEVEL;
	if (masked)
		e |= IOAPIC_RED_MASKED;
	e |= (uint64_t)(dest & 0xff) << IOAPIC_DEST_SHIFT;
	return (e);
}

uint32_t	ioapic_count_from_ver(uint32_t ver)
{
	uint32_t	n;

	n = ((ver >> 16) & 0xff) + 1;
	if (n > IOAPIC_MAX_PINS)
		n = IOAPIC_MAX_PINS;
	return (n);
}
