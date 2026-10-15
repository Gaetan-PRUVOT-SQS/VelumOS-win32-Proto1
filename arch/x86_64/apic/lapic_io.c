#include "apic_int.h"
#include "velum/msr.h"

static t_lapic_state	g_lapic;

void	lapic_set_mode(bool x2, volatile uint32_t *mmio)
{
	g_lapic.x2 = x2;
	g_lapic.mmio = mmio;
	g_lapic.ready = true;
}

bool	lapic_ready(void)
{
	return (g_lapic.ready);
}

bool	lapic_is_x2(void)
{
	return (g_lapic.x2);
}

uint32_t	lapic_read(uint32_t reg)
{
	if (g_lapic.x2)
		return ((uint32_t)msr_read(X2APIC_MSR_BASE + (reg >> 4)));
	return (g_lapic.mmio[reg / 4]);
}

void	lapic_write(uint32_t reg, uint32_t val)
{
	if (g_lapic.x2)
	{
		msr_write(X2APIC_MSR_BASE + (reg >> 4), val);
		return ;
	}
	g_lapic.mmio[reg / 4] = val;
}
