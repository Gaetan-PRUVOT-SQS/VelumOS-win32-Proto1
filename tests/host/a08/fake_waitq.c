#include "fake.h"

int	fk_hook_run(int forced)
{
	int	r;

	if (g_fk.in_hook || !g_fk.hook)
		return (0);
	g_fk.in_hook = 1;
	r = g_fk.hook(g_fk.hook_ctx, forced);
	g_fk.in_hook = 0;
	return (r);
}

static int	fk_wait_expire(t_waitq *wq, uint64_t timeout)
{
	wq->head = NULL;
	if (g_fk.hook)
	{
		g_fk.slept_at_end++;
		return (g_fk.wait_end_rc);
	}
	if (timeout == TIMEOUT_NONE)
		return (g_fk.wait_end_rc);
	g_fk.now += timeout;
	return (E_TIMEOUT);
}

int	waitq_wait(t_waitq *wq, t_spinlock *held, uint64_t timeout)
{
	int	run;
	int	rc;

	wq->head = (struct s_thread *)&g_fk;
	held->ticket = 0;
	g_fk.depth--;
	g_fk.waits++;
	run = 1;
	while (wq->head && g_fk.hook && run)
		run = fk_hook_run(1);
	rc = 0;
	if (wq->head)
		rc = fk_wait_expire(wq, timeout);
	held->ticket = 1;
	g_fk.depth++;
	return (rc);
}
