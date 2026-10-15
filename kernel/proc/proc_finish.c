#include "proc_int.h"
#include "velum/klog.h"

static void	finish_wait_gone(t_thread *t)
{
	uint32_t	waited;

	thread_join(t, TIMEOUT_NONE);
	waited = 0;
	while (__atomic_load_n(&t->state, __ATOMIC_ACQUIRE) != TS_ZOMBIE
		&& waited < 20000)
	{
		sched_sleep_ns(100000);
		waited++;
	}
}

static void	finish_threads(t_process *p)
{
	t_procext	*x;
	uint32_t	i;

	x = proc_ext(p);
	i = 0;
	while (i < PROC_THREADS_MAX)
	{
		if (x->th[i].state != PT_FREE && x->th[i].t)
		{
			finish_wait_gone(x->th[i].t);
			proc_thread_release(x->th[i].t);
		}
		x->th[i].t = NULL;
		x->th[i].state = PT_FREE;
		i++;
	}
}

static void	finish_memory(t_process *p)
{
	t_aspace	*as;
	void		*ht;
	uint64_t	fl;

	if (object_process_cleanup)
		object_process_cleanup(p);
	fl = spin_lock_irqsave(&p->lock);
	as = p->aspace;
	p->aspace = NULL;
	ht = p->handles;
	p->handles = NULL;
	spin_unlock_irqrestore(&p->lock, fl);
	if (ht && handle_table_destroy)
		handle_table_destroy(ht);
	if (as)
		vmm_aspace_destroy(as);
}

static void	finish_orphans(uint32_t pid)
{
	t_proctab	*tab;
	uint64_t	fl;
	uint32_t	i;
	uint32_t	heir;

	tab = proc_tab();
	fl = spin_lock_irqsave(&tab->lock);
	heir = tab->init_pid;
	if (heir == pid)
		heir = 0;
	i = 0;
	while (i < PROC_MAX)
	{
		if (tab->slot[i] && tab->slot[i]->ppid == pid)
			__atomic_store_n(&tab->slot[i]->ppid, heir, __ATOMIC_RELAXED);
		i++;
	}
	spin_unlock_irqrestore(&tab->lock, fl);
}

void	proc_finish(t_process *p)
{
	t_object	*o;
	uint64_t	fl;

	finish_threads(p);
	fl = spin_lock_irqsave(&p->lock);
	__atomic_store_n(&p->state, PS_ZOMBIE, __ATOMIC_RELEASE);
	spin_unlock_irqrestore(&p->lock, fl);
	finish_memory(p);
	finish_orphans(p->pid);
	klog_debug("proc: %s (%u) terminé, code %d", p->name, p->pid,
		p->exit_code);
	fl = spin_lock_irqsave(&p->lock);
	o = p->obj;
	p->obj = NULL;
	spin_unlock_irqrestore(&p->lock, fl);
	if (o && obj_task_exited)
		obj_task_exited(o);
	if (o)
		obj_unref(o);
	proc_unref(p);
}
