#include "obj_int.h"

static uint64_t	tmr_next(const t_tmrslot *s)
{
	uint64_t	now;
	uint64_t	next;

	now = time_now_ns();
	if (s->period > TIMEOUT_INF - s->deadline)
		return (0);
	next = s->deadline + s->period;
	if (next > now)
		return (next);
	if (s->period > TIMEOUT_INF - now)
		return (0);
	return (now + s->period);
}

static int	tmr_arm(uint32_t slot, uint64_t gen, uint64_t deadline)
{
	int64_t		id;
	uint64_t	fl;
	bool		stale;

	id = timer_arm(deadline, tmr_fire,
			(void *)(uintptr_t)((gen << TMR_SLOT_BITS) | slot));
	if (id <= 0)
		return (E_NOMEM);
	fl = spin_lock_irqsave(&g_tmr.lock);
	stale = (g_tmr.slots[slot].armgen != gen);
	if (!stale)
		g_tmr.slots[slot].tid = id;
	spin_unlock_irqrestore(&g_tmr.lock, fl);
	if (stale)
		timer_cancel(id);
	return (0);
}

void	tmr_fire(void *ctx)
{
	uint32_t	slot;
	uint64_t	gen;
	uint64_t	fl;
	uint64_t	next;
	t_tmrslot	*s;

	slot = (uint32_t)((uintptr_t)ctx & TMR_SLOT_MASK);
	gen = (uint64_t)(uintptr_t)ctx >> TMR_SLOT_BITS;
	next = 0;
	fl = spin_lock_irqsave(&g_tmr.lock);
	s = &g_tmr.slots[slot];
	if (slot < TMR_SLOTS && s->obj && s->armgen == gen)
	{
		s->tid = 0;
		sig_set(s->obj, 1);
		if (s->period)
			next = tmr_next(s);
		if (next)
			s->deadline = next;
	}
	spin_unlock_irqrestore(&g_tmr.lock, fl);
	if (next)
		tmr_arm(slot, gen, next);
}

static uint64_t	tmr_rearm_slot(uint32_t slot, uint64_t deadline,
		uint64_t period, int64_t *old)
{
	t_tmrslot	*s;
	uint64_t	fl;
	uint64_t	gen;

	fl = spin_lock_irqsave(&g_tmr.lock);
	s = &g_tmr.slots[slot];
	s->armgen++;
	gen = s->armgen;
	*old = s->tid;
	s->tid = 0;
	s->deadline = deadline;
	s->period = period;
	spin_unlock_irqrestore(&g_tmr.lock, fl);
	return (gen);
}

int	tmr_set(t_object *o, uint64_t deadline, uint64_t period)
{
	t_tmrimpl	*t;
	uint64_t	gen;
	int64_t		old;

	if (!o || o->type != OBJ_TIMER || !sig_is(o))
		return (E_INVAL);
	if (period != 0 && period < TMR_PERIOD_MIN)
		return (E_INVAL);
	t = o->impl;
	if (t->slot >= TMR_SLOTS)
		return (E_INVAL);
	sig_set(o, 0);
	gen = tmr_rearm_slot(t->slot, deadline, period, &old);
	if (old > 0)
		timer_cancel(old);
	if (deadline == 0)
		return (0);
	return (tmr_arm(t->slot, gen, deadline));
}
