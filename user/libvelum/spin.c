#include "velum/vsync.h"
#include "velum/vtime.h"

#define SPIN_TRIES 64

void	v_spin_lock(t_vspin *spin)
{
	uint32_t	tries;

	tries = 0;
	while (__atomic_exchange_n(&spin->locked, 1, __ATOMIC_ACQUIRE))
	{
		tries++;
		if (tries < SPIN_TRIES)
			__builtin_ia32_pause();
		else
			v_yield();
	}
}

int	v_spin_trylock(t_vspin *spin)
{
	return (!__atomic_exchange_n(&spin->locked, 1, __ATOMIC_ACQUIRE));
}

void	v_spin_unlock(t_vspin *spin)
{
	__atomic_store_n(&spin->locked, 0, __ATOMIC_RELEASE);
}
