#include "display_int.h"
#include "velum/irqflags.h"

bool	display_try_lock(void)
{
	return (__atomic_exchange_n(&g_display.busy, 1, __ATOMIC_ACQUIRE) == 0);
}

void	display_unlock(void)
{
	__atomic_store_n(&g_display.busy, 0, __ATOMIC_RELEASE);
}

bool	display_lock_wait(void)
{
	uint32_t	spins;

	spins = 0;
	while (spins < DISP_LOCK_SPINS)
	{
		if (display_try_lock())
			return (true);
		cpu_relax();
		spins++;
	}
	return (false);
}

t_dspstate	display_state(void)
{
	return (g_display.state);
}

void	display_state_set(t_dspstate st)
{
	bool	held;

	held = display_lock_wait();
	g_display.state = st;
	if (held)
		display_unlock();
}
