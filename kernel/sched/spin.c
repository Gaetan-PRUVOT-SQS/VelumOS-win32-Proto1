#include "sched_int.h"
#include "velum/irqflags.h"
#include "velum/panic.h"
#include "velum/timer.h"

static void	spin_stuck_check(t_spinlock *l, uint64_t *start, void *caller)
{
	uint64_t	now;

	now = time_now_ns();
	if (*start == 0 || now < *start)
	{
		*start = now;
		return ;
	}
	if (now - *start > SPIN_STUCK_NS)
		panic("spin: interblocage sur '%s' (appelant %p)", l->name, caller);
}

void	spin_acquire(t_spinlock *l, void *caller)
{
	uint32_t	mine;
	uint32_t	spins;
	uint64_t	start;

	if (SCHED_DEBUG)
		lockdep_acquire(l, caller, true);
	mine = __atomic_fetch_add(&l->ticket, 1, __ATOMIC_RELAXED);
	spins = 0;
	start = 0;
	while (__atomic_load_n(&l->serving, __ATOMIC_ACQUIRE) != mine)
	{
		cpu_relax();
		spins++;
		if (SCHED_DEBUG && (spins & SPIN_CHECK_MASK) == 0)
			spin_stuck_check(l, &start, caller);
	}
}

void	spin_init(t_spinlock *l, const char *name)
{
	l->ticket = 0;
	l->serving = 0;
	l->name = name;
	if (!name)
		l->name = "?";
}

void	spin_lock(t_spinlock *l)
{
	sched_preempt_disable();
	spin_acquire(l, __builtin_return_address(0));
}

bool	spin_trylock(t_spinlock *l)
{
	uint32_t	cur;

	sched_preempt_disable();
	cur = __atomic_load_n(&l->serving, __ATOMIC_RELAXED);
	if (__atomic_compare_exchange_n(&l->ticket, &cur, cur + 1, false,
			__ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
	{
		if (SCHED_DEBUG)
			lockdep_acquire(l, __builtin_return_address(0), false);
		return (true);
	}
	sched_preempt_enable();
	return (false);
}
