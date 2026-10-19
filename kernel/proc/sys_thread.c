#include "proc_int.h"
#include "proc_sys.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/util.h"

int64_t	sys_exit(const t_sysargs *a)
{
	proc_exit_current((int)a->a[0]);
}

int64_t	sys_thread_exit(const t_sysargs *a)
{
	proc_thread_leave(proc_current(), sched_current(), (int)a->a[0]);
	thread_exit((int)a->a[0]);
}

static void	thread_req(t_threadreq *rq, t_process *p, const t_sysargs *a,
				const t_pthread *st)
{
	memset(rq, 0, sizeof(*rq));
	rq->name = p->name;
	rq->prio = (int32_t)a->a[3];
	rq->flags = THREAD_USER;
	rq->proc = p;
	rq->entry = a->a[0];
	rq->arg = (void *)a->a[1];
	rq->sp = st->stack_va + st->stack_len - 2 * sizeof(uint64_t);
}

static int64_t	thread_result(t_process *p, t_thread *t)
{
	t_handle	h;
	int64_t		rc;

	rc = t->tid;
	if (t->obj && handle_alloc)
	{
		rc = handle_alloc(p, t->obj, THREAD_HRIGHTS, &h);
		if (rc == 0)
			rc = h;
	}
	thread_unref(t);
	return (rc);
}

static int	thread_stack_budget(const t_process *p, uint64_t size)
{
	uint64_t	len;

	if (p->mem_limit == 0 || size > TSTACK_MAX)
		return (0);
	len = size;
	if (len == 0)
		len = TSTACK_DEFAULT;
	if (len < TSTACK_MIN)
		return (0);
	len = align_up(len, PAGE_SIZE);
	if (vmm_pages_used(p->aspace) * PAGE_SIZE + len > p->mem_limit)
		return (E_NOMEM);
	return (0);
}

int64_t	sys_thread_create(const t_sysargs *a)
{
	t_threadreq	rq;
	t_pthread	st;
	t_process	*p;
	t_thread	*t;
	int			rc;

	p = proc_current();
	if (!p || a->a[4] != 0 || a->a[0] < USER_MIN || a->a[0] >= USER_TOP)
		return (E_INVAL);
	memset(&st, 0, sizeof(st));
	rc = syscall_prio_check(a->a[3]);
	if (rc == 0)
		rc = thread_stack_budget(p, a->a[2]);
	if (rc == 0)
		rc = proc_tstack_alloc(p, a->a[2], a->a[1], &st);
	if (rc < 0)
		return (rc);
	thread_req(&rq, p, a, &st);
	t = proc_thread_add(p, &rq, &st);
	if (t)
		return (thread_result(p, t));
	if (vmm_stack_unmap(p->aspace, st.stack_va, st.stack_len - PAGE_SIZE) < 0)
		klog_warn("proc: pile de fil %#llx non rendue",
			(unsigned long long)st.stack_va);
	return (E_AGAIN);
}
