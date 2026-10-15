#include "proc_int.h"
#include "velum/irqflags.h"

void	proc_mark_exiting(t_process *p, int code)
{
	uint64_t	fl;

	fl = spin_lock_irqsave(&p->lock);
	if (p->state == PS_RUNNING)
	{
		p->exit_code = code;
		__atomic_store_n(&p->state, PS_EXITING, __ATOMIC_RELEASE);
	}
	spin_unlock_irqrestore(&p->lock, fl);
}

static void	proc_cancel_thread(t_thread *t)
{
	if (thread_cancel)
		thread_cancel(t);
	else if (t->state == TS_BLOCKED)
		sched_wake(t);
}

static uint32_t	collect_others(t_process *p, t_thread **list)
{
	t_procext	*x;
	uint64_t	fl;
	uint32_t	i;
	uint32_t	n;

	x = proc_ext(p);
	n = 0;
	fl = spin_lock_irqsave(&p->lock);
	i = 0;
	while (i < PROC_THREADS_MAX)
	{
		if (x->th[i].state == PT_LIVE && x->th[i].t != sched_current())
		{
			thread_ref(x->th[i].t);
			list[n] = x->th[i].t;
			n++;
		}
		i++;
	}
	spin_unlock_irqrestore(&p->lock, fl);
	return (n);
}

void	proc_wake_threads(t_process *p)
{
	t_thread	*list[PROC_THREADS_MAX];
	uint32_t	i;
	uint32_t	n;

	n = collect_others(p, list);
	i = 0;
	while (i < n)
	{
		proc_cancel_thread(list[i]);
		thread_unref(list[i]);
		i++;
	}
}

void	proc_kill(t_process *p, int code)
{
	if (!p)
		return ;
	if (p == proc_current())
		proc_exit_current(code);
	proc_mark_exiting(p, code);
	proc_wake_threads(p);
}
