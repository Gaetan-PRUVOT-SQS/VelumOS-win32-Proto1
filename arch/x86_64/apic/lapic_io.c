#include "apic_int.h"
#include "velum/err.h"
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

static int	icr_wait_idle(void)
{
	uint32_t	spins;

	spins = 0;
	while (lapic_read(LAPIC_ICR_LO) & LAPIC_ICR_PENDING)
	{
		if (++spins > LAPIC_ICR_SPINS)
			return (E_TIMEOUT);
		__builtin_ia32_pause();
	}
	return (E_OK);
}

int	lapic_icr_send(uint32_t dest, uint32_t lo)
{
	if (!g_lapic.ready)
		return (E_NOENT);
	if (g_lapic.x2)
	{
		msr_write(X2APIC_MSR_BASE + (LAPIC_ICR_LO >> 4),
			lapic_icr_x2(lo, dest));
		return (E_OK);
	}
	if (dest > 0xff)
		return (E_INVAL);
	if (icr_wait_idle() < 0)
		return (E_TIMEOUT);
	g_lapic.mmio[LAPIC_ICR_HI / 4] = dest << LAPIC_ICR_DEST_SHIFT;
	g_lapic.mmio[LAPIC_ICR_LO / 4] = lo;
	return (icr_wait_idle());
}
