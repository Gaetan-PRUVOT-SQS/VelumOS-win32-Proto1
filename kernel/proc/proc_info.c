#include "proc_int.h"
#include "velum/libk.h"
#include "velum/util.h"

t_process	*proc_current(void)
{
	t_thread	*t;

	t = sched_current();
	if (!t)
		return (NULL);
	return (t->proc);
}

static uint64_t	proc_cpu_time(t_process *p)
{
	t_procext	*x;
	uint64_t	total;
	uint32_t	i;

	x = proc_ext(p);
	total = 0;
	i = 0;
	while (i < PROC_THREADS_MAX)
	{
		if (x->th[i].t && x->th[i].state != PT_FREE)
			total += x->th[i].t->cpu_time_ns;
		i++;
	}
	return (total);
}

void	proc_fill_info(t_process *p, t_procinfo *out)
{
	uint64_t	fl;

	memset(out, 0, sizeof(*out));
	fl = spin_lock_irqsave(&p->lock);
	out->pid = p->pid;
	out->ppid = p->ppid;
	out->state = p->state;
	out->flags = p->flags;
	out->nthreads = p->nthreads;
	out->reserved = (uint32_t)p->exit_code;
	if (p->aspace)
		out->mem_bytes = vmm_pages_used(p->aspace) * PAGE_SIZE;
	out->cpu_time_ns = proc_cpu_time(p);
	out->started_ns = p->created_ns;
	memcpy(out->name, p->name, sizeof(out->name));
	spin_unlock_irqrestore(&p->lock, fl);
}

int	proc_list(t_procinfo *out, uint32_t max)
{
	t_proctab	*tab;
	uint64_t	fl;
	uint32_t	i;
	uint32_t	n;

	tab = proc_tab();
	n = 0;
	fl = spin_lock_irqsave(&tab->lock);
	i = 0;
	while (i < PROC_MAX && n < max)
	{
		if (tab->slot[i])
		{
			proc_fill_info(tab->slot[i], &out[n]);
			n++;
		}
		i++;
	}
	spin_unlock_irqrestore(&tab->lock, fl);
	return ((int)n);
}

void	proc_free(t_process *p)
{
	if (p->handles && handle_table_destroy)
		handle_table_destroy(p->handles);
	p->handles = NULL;
	if (p->aspace)
		vmm_aspace_destroy(p->aspace);
	p->aspace = NULL;
	kfree(p);
}
