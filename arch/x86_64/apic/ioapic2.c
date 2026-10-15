#include "apic_int.h"
#include "../../../kernel/irq/a05_lock.h"
#include "velum/err.h"

static t_a05lock	g_ioapic_lock = {0, "ioapic"};

bool	ioapic_covers(uint32_t gsi)
{
	uint32_t	pin;

	return (ioapic_find(gsi, &pin) != NULL);
}

uint32_t	ioapic_gsi_end(void)
{
	uint32_t		gsi;
	uint32_t		pin;
	uint32_t		end;

	gsi = 0;
	end = 0;
	while (gsi < ACPI_MAX_IOAPICS * IOAPIC_MAX_PINS)
	{
		if (ioapic_find(gsi, &pin))
			end = gsi + 1;
		gsi++;
	}
	return (end);
}

int	ioapic_route(uint32_t gsi, uint8_t vec, uint32_t trig, uint32_t dest)
{
	t_ioapic_dev	*d;
	uint32_t		pin;
	uint64_t		e;
	uint64_t		fl;

	d = ioapic_find(gsi, &pin);
	if (!d)
		return (E_INVAL);
	if (dest > 0xff)
		return (E_NOTSUP);
	e = ioapic_entry(vec, trig, dest, false);
	fl = a05_lock(&g_ioapic_lock);
	ioapic_wr(d, IOAPIC_REG_RED + 2 * pin, IOAPIC_RED_MASKED);
	ioapic_wr(d, IOAPIC_REG_RED + 2 * pin + 1, (uint32_t)(e >> 32));
	ioapic_wr(d, IOAPIC_REG_RED + 2 * pin, (uint32_t)e);
	a05_unlock(&g_ioapic_lock, fl);
	return (E_OK);
}

int	ioapic_set_mask(uint32_t gsi, bool masked)
{
	t_ioapic_dev	*d;
	uint32_t		pin;
	uint32_t		low;
	uint64_t		fl;

	d = ioapic_find(gsi, &pin);
	if (!d)
		return (E_INVAL);
	fl = a05_lock(&g_ioapic_lock);
	low = ioapic_rd(d, IOAPIC_REG_RED + 2 * pin);
	if (masked)
		low |= IOAPIC_RED_MASKED;
	else
		low &= ~(uint32_t)IOAPIC_RED_MASKED;
	ioapic_wr(d, IOAPIC_REG_RED + 2 * pin, low);
	a05_unlock(&g_ioapic_lock, fl);
	return (E_OK);
}
