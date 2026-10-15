#include "proc_int.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/panic.h"
#include "velum/util.h"

static void	leave_unlink(t_process *p, t_thread *t)
{
	t_thread	**link;

	link = &p->threads;
	while (*link && *link != t)
		link = &(*link)->proc_next;
	if (*link)
		*link = t->proc_next;
	t->proc_next = NULL;
}

static int	leave_locked(t_process *p, t_thread *t, t_pthread *out)
{
	t_procext	*x;
	uint32_t	i;

	x = proc_ext(p);
	memset(out, 0, sizeof(*out));
	i = 0;
	while (i < PROC_THREADS_MAX
		&& !(x->th[i].state == PT_LIVE && x->th[i].t == t))
		i++;
	if (i < PROC_THREADS_MAX)
	{
		*out = x->th[i];
		x->th[i].state = PT_DEAD;
		x->th[i].stack_len = 0;
		p->nthreads--;
		leave_unlink(p, t);
		return (p->nthreads == 0);
	}
	return (0);
}

void	proc_thread_leave(t_process *p, t_thread *t, int code)
{
	t_pthread	slot;
	uint64_t	fl;
	int			last;

	fl = spin_lock_irqsave(&p->lock);
	last = leave_locked(p, t, &slot);
	if (last && p->state == PS_RUNNING)
	{
		p->exit_code = code;
		__atomic_store_n(&p->state, PS_EXITING, __ATOMIC_RELEASE);
	}
	spin_unlock_irqrestore(&p->lock, fl);
	if (t->obj && obj_task_exited)
		obj_task_exited(t->obj);
	if (!last && slot.stack_len && p->aspace && vmm_stack_unmap(p->aspace,
			slot.stack_va, slot.stack_len - PAGE_SIZE) < 0)
		klog_warn("proc: pile du fil %u non rendue", t->tid);
	__atomic_store_n(&t->aspace, vmm_kernel_aspace(), __ATOMIC_RELEASE);
	if (last)
		reap_enqueue(p);
}

_Noreturn void	proc_exit_current(int code)
{
	t_process	*p;
	t_thread	*t;

	t = sched_current();
	p = proc_current();
	kassert_check(p != NULL, "proc: sortie d'un fil sans processus");
	proc_mark_exiting(p, code);
	proc_wake_threads(p);
	proc_thread_leave(p, t, code);
	thread_exit(code);
}

void	proc_return_check(void)
{
	t_process	*p;

	p = proc_current();
	if (p && __atomic_load_n(&p->state, __ATOMIC_ACQUIRE) != PS_RUNNING)
		proc_exit_current(p->exit_code);
}
