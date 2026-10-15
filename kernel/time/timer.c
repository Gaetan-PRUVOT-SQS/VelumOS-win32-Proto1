#include "time_int.h"
#include "velum/sched.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "velum/err.h"
#include "velum/irq.h"

#define LAPIC_COUNT_MAX 0xffffffffull

static t_timers	g_timers;

t_timers	*timers_state(void)
{
	return (&g_timers);
}

static void	hw_program(uint64_t deadline)
{
	uint64_t	now;
	uint64_t	ticks;

	if (g_timers.mode == TIMER_MODE_DEADLINE)
	{
		if (deadline == UINT64_MAX)
			lapic_timer_set_deadline(0);
		else
			lapic_timer_set_deadline(clock_tsc_at(deadline));
		return ;
	}
	if (deadline == UINT64_MAX)
	{
		lapic_timer_set_count(0);
		return ;
	}
	now = time_now_ns();
	ticks = 1;
	if (deadline > now)
		ticks = clockconv_apply(&g_timers.lapic_per_ns, deadline - now) + 1;
	if (ticks > LAPIC_COUNT_MAX)
		ticks = LAPIC_COUNT_MAX;
	lapic_timer_set_count((uint32_t)ticks);
}

void	timer_irq(t_regs *regs, void *ctx)
{
	t_tslot		t;
	uint64_t	fl;

	(void)regs;
	(void)ctx;
	fl = a05_lock(&g_timers.lock);
	while (theap_pop(&g_timers.heap, time_now_ns(), &t))
	{
		a05_unlock(&g_timers.lock, fl);
		t.fn(t.ctx);
		fl = a05_lock(&g_timers.lock);
	}
	hw_program(theap_peek(&g_timers.heap));
	a05_unlock(&g_timers.lock, fl);
	apic_eoi();
	sched_irq_exit();
}

int64_t	timer_arm(uint64_t deadline_ns, t_timerfn fn, void *ctx)
{
	uint64_t	fl;
	int64_t		id;

	if (!fn)
		return (E_INVAL);
	if (!g_timers.ready)
		return (E_NODEV);
	fl = a05_lock(&g_timers.lock);
	id = theap_insert(&g_timers.heap, deadline_ns, fn, ctx);
	if (id > 0 && theap_peek(&g_timers.heap) == deadline_ns)
		hw_program(deadline_ns);
	a05_unlock(&g_timers.lock, fl);
	return (id);
}

bool	timer_cancel(int64_t id)
{
	uint64_t	fl;
	bool		done;

	if (!g_timers.ready)
		return (false);
	fl = a05_lock(&g_timers.lock);
	done = theap_cancel(&g_timers.heap, id);
	a05_unlock(&g_timers.lock, fl);
	return (done);
}
