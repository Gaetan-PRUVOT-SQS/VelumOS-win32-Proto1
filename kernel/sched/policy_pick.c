#include "sched_int.h"

void	policy_requeue(t_runq *rq, t_kthread *kt, int why)
{
	bool	expired;

	expired = (kt->t.quantum_left == 0);
	if (expired)
	{
		kt->boost = 0;
		kt->t.prio = policy_prio(kt);
	}
	kt->t.state = TS_READY;
	runq_push(rq, kt, !(why == SR_YIELD || expired));
}

t_kthread	*policy_pick(t_runq *rq, t_kthread *prev, int why, t_kthread *idle)
{
	t_kthread	*next;

	if (prev != idle && prev->t.state == TS_RUNNING)
		policy_requeue(rq, prev, why);
	next = runq_pop_top(rq);
	if (!next)
		return (idle);
	return (next);
}

bool	policy_tick(t_runq *rq, t_kthread *cur, uint64_t now,
			uint64_t *slice_start)
{
	int32_t	aged;

	policy_account(cur, slice_start, now);
	aged = policy_age(rq, now);
	if (cur->kflags & KT_IDLE)
		return (rq->count > 0);
	if (cur->t.quantum_left == 0 || aged > cur->t.prio)
		return (true);
	return (rq->count > 0);
}
