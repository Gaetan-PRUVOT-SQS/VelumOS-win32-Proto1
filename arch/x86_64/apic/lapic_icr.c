#include "apic_int.h"

uint32_t	lapic_icr_value(uint32_t mode, uint8_t vec)
{
	uint32_t	lo;

	lo = mode | LAPIC_ICR_ASSERT | vec;
	if (mode == LAPIC_ICR_INIT)
		lo |= LAPIC_ICR_LEVEL;
	return (lo);
}

uint64_t	lapic_icr_x2(uint32_t lo, uint32_t dest)
{
	return (((uint64_t)dest << 32) | lo);
}
