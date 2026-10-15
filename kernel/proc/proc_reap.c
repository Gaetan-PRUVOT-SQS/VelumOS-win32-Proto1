#include "proc_int.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/panic.h"

static t_reapq	g_reap;

static t_process	*reap_pop(void)
{
	t_process	*p;
	uint64_t	fl;

	p = NULL;
	fl = spin_lock_irqsave(&g_reap.lock);
	if (g_reap.n > 0)
	{
		p = g_reap.q[g_reap.head];
		g_reap.q[g_reap.head] = NULL;
		g_reap.head = (g_reap.head + 1) % PROC_MAX;
		g_reap.n--;
	}
	spin_unlock_irqrestore(&g_reap.lock, fl);
	return (p);
}

static void	reaper_main(void *arg)
{
	t_process	*p;

	(void)arg;
	while (1)
	{
		event_wait(&g_reap.ev, TIMEOUT_NONE);
		p = reap_pop();
		while (p)
		{
			proc_finish(p);
			p = reap_pop();
		}
	}
}

int	reap_init(void)
{
	t_threadreq	rq;

	spin_init(&g_reap.lock, "reapq");
	event_init(&g_reap.ev, false, false);
	memset(&rq, 0, sizeof(rq));
	rq.name = "reaper";
	rq.fn = reaper_main;
	rq.prio = PRIO_HIGH;
	g_reap.thread = thread_create(&rq);
	if (!g_reap.thread)
		return (E_NOMEM);
	return (0);
}

void	reap_enqueue(t_process *p)
{
	uint64_t	fl;
	int			full;

	fl = spin_lock_irqsave(&g_reap.lock);
	full = (g_reap.n >= PROC_MAX);
	if (!full)
	{
		g_reap.q[(g_reap.head + g_reap.n) % PROC_MAX] = p;
		g_reap.n++;
	}
	spin_unlock_irqrestore(&g_reap.lock, fl);
	kassert_check(!full, "proc: file du moissonneur pleine");
	event_set(&g_reap.ev);
}
