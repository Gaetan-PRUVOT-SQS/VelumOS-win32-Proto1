#include "fake_sched.h"

void	sched_park(void)
{
	t_kthread	*kt;

	kt = sched_kself();
	fh_lock();
	g_ft.parks++;
	while (!kt->token)
	{
		kt->t.state = TS_BLOCKED;
		fh_wait_cond();
	}
	kt->token = 0;
	kt->t.state = TS_RUNNING;
	fh_unlock();
}

void	sched_unpark(t_kthread *kt, bool boost)
{
	(void)boost;
	fh_lock();
	kt->token = 1;
	fh_broadcast();
	fh_unlock();
}

void	sched_park_prepare(t_kthread *kt)
{
	fh_lock();
	kt->token = 0;
	fh_unlock();
}
