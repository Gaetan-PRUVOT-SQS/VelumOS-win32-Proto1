#include "sched_int.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/libk.h"
#include "velum/proc.h"
#include "velum/util.h"
#include "velum/vmm.h"

static bool	thread_req_ok(const t_threadreq *rq)
{
	if (!rq || rq->prio < 0 || rq->prio > PRIO_MAX)
		return (false);
	if (rq->flags & ~(uint32_t)(THREAD_USER | THREAD_DETACHED))
		return (false);
	if (!(rq->flags & THREAD_USER))
		return (rq->fn != NULL);
	if (!rq->proc || !rq->proc->aspace)
		return (false);
	return (rq->entry >= USER_MIN && rq->entry < USER_TOP
		&& rq->sp > USER_MIN && rq->sp <= USER_TOP);
}

static void	thread_fill(t_kthread *kt, const t_threadreq *rq)
{
	kt->t.tid = __atomic_fetch_add(&g_sched.next_tid, 1, __ATOMIC_RELAXED);
	if (rq->name)
		strlcpy(kt->t.name, rq->name, sizeof(kt->t.name));
	else
		strlcpy(kt->t.name, "fil", sizeof(kt->t.name));
	kt->t.state = TS_NEW;
	kt->t.prio = rq->prio;
	kt->t.base_prio = rq->prio;
	kt->t.flags = rq->flags;
	kt->t.proc = rq->proc;
	kt->t.refs = 2;
	if (rq->flags & THREAD_DETACHED)
		kt->t.refs = 1;
	kt->fn = rq->fn;
	kt->arg = rq->arg;
	kt->entry = rq->entry;
	kt->user_sp = rq->sp;
	if (rq->flags & THREAD_USER)
		kt->t.aspace = rq->proc->aspace;
	waitq_init(&kt->t.joiners);
}

static int	thread_resources(t_kthread *kt)
{
	void	*stack;

	stack = vmm_kstack_alloc(KSTACK_PAGES);
	if (!stack)
		return (E_NOMEM);
	kt->t.kstack_base = (uint64_t)stack;
	kt->t.kstack_top = kt->t.kstack_base + KSTACK_PAGES * PAGE_SIZE;
	if (!(kt->t.flags & THREAD_USER))
		return (0);
	kt->t.fpu = kmalloc_aligned(cpu_fpu_area_size(), 64);
	if (!kt->t.fpu)
	{
		vmm_kstack_free(stack, KSTACK_PAGES);
		kt->t.kstack_base = 0;
		return (E_NOMEM);
	}
	cpu_fpu_init_state(kt->t.fpu);
	return (0);
}

t_kthread	*thread_alloc(const t_threadreq *rq)
{
	t_kthread	*kt;

	if (!thread_req_ok(rq))
		return (NULL);
	kt = kmalloc_tag(sizeof(*kt), HEAP_SCHED);
	if (!kt)
		return (NULL);
	memset(kt, 0, sizeof(*kt));
	thread_fill(kt, rq);
	if (thread_resources(kt) < 0)
	{
		kfree(kt);
		return (NULL);
	}
	arch_thread_setup(kt);
	return (kt);
}

t_thread	*thread_create(const t_threadreq *rq)
{
	t_kthread	*kt;

	kt = thread_alloc(rq);
	if (!kt)
		return (NULL);
	sched_unpark(kt, false);
	return (&kt->t);
}
