#include "sched_int.h"

int32_t	policy_prio(const t_kthread *kt)
{
	int32_t	p;

	p = kt->t.base_prio;
	if (kt->boost > p)
		p = kt->boost;
	if (kt->inherit > p)
		p = kt->inherit;
	if (p > PRIO_MAX)
		p = PRIO_MAX;
	return (p);
}

void	policy_wake_boost(t_kthread *kt)
{
	int32_t	b;

	if (kt->t.base_prio >= PRIO_REALTIME || (kt->kflags & KT_IDLE))
		return ;
	b = kt->t.base_prio + BOOST_WAKE;
	if (b > PRIO_AGED)
		b = PRIO_AGED;
	if (b > kt->boost)
		kt->boost = b;
}

bool	policy_should_preempt(const t_kthread *cur, const t_kthread *woken)
{
	if (cur->kflags & KT_IDLE)
		return (true);
	return (woken->t.prio > cur->t.prio);
}

void	policy_account(t_kthread *cur, uint64_t *slice_start, uint64_t now)
{
	uint64_t	d;

	if (now <= *slice_start)
		return ;
	d = now - *slice_start;
	cur->t.cpu_time_ns += d;
	if (d >= cur->t.quantum_left)
		cur->t.quantum_left = 0;
	else
		cur->t.quantum_left -= (uint32_t)d;
	*slice_start = now;
}

int32_t	policy_age(t_runq *rq, uint64_t now)
{
	int32_t		p;
	int32_t		best;
	t_kthread	*kt;

	best = -1;
	p = 0;
	while (p < PRIO_AGED)
	{
		kt = rq->head[p];
		if (kt && now >= kt->ready_ns && now - kt->ready_ns >= AGING_NS)
		{
			runq_remove(rq, kt);
			kt->boost = PRIO_AGED;
			kt->t.prio = policy_prio(kt);
			runq_push(rq, kt, false);
			best = kt->t.prio;
		}
		p++;
	}
	return (best);
}
