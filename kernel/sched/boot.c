#include "sched_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/timer.h"

static void	cpusched_init(t_cpusched *cs, t_cpu *cpu)
{
	spin_init(&cs->lock, "runq");
	runq_init(&cs->rq);
	cs->cpu = cpu;
	if (cs->quantum_ns == 0)
		cs->quantum_ns = QUANTUM_NS;
	cs->active_as = vmm_kernel_aspace();
	cs->slice_start = time_now_ns();
}

static void	boot_thread_init(t_kthread *kt)
{
	kt->t.tid = 0;
	strlcpy(kt->t.name, "kmain", sizeof(kt->t.name));
	kt->t.state = TS_RUNNING;
	kt->t.prio = PRIO_NORMAL;
	kt->t.base_prio = PRIO_NORMAL;
	kt->t.refs = 1;
	kt->t.quantum_left = QUANTUM_NS;
	kt->t.kstack_base = arch_boot_stack_bottom();
	kt->t.kstack_top = arch_boot_stack_top();
	kt->kflags = KT_STATIC;
	waitq_init(&kt->t.joiners);
}

static t_kthread	*boot_spawn(const char *name, t_threadfn fn, int32_t prio,
						void *arg)
{
	t_threadreq	rq;

	memset(&rq, 0, sizeof(rq));
	rq.name = name;
	rq.fn = fn;
	rq.arg = arg;
	rq.prio = prio;
	rq.flags = THREAD_DETACHED;
	return (thread_alloc(&rq));
}

int	sched_boot_init(void)
{
	t_cpusched	*cs;
	t_cpu		*cpu;

	cs = &g_sched.bsp;
	cpu = cpu_self();
	cpusched_init(cs, cpu);
	boot_thread_init(&g_sched.boot);
	cpu->sched = cs;
	cpu->current = &g_sched.boot.t;
	cpu->preempt = 0;
	g_sched.next_tid = 1;
	__atomic_store_n(&g_sched.ready, 1, __ATOMIC_RELEASE);
	cs->idle = boot_spawn("idle", sched_idle_main, PRIO_IDLE, cs);
	cs->reaper = boot_spawn("reaper", sched_reaper_main, PRIO_HIGH, cs);
	if (!cs->idle || !cs->reaper)
		return (E_NOMEM);
	cs->idle->kflags |= KT_IDLE;
	cs->idle->t.state = TS_READY;
	sched_unpark(cs->reaper, false);
	sched_syscalls_register();
	klog_info("sched: prêt, quantum %llu us, fils inactif %u et faucheur %u",
		cs->quantum_ns / 1000, cs->idle->t.tid, cs->reaper->t.tid);
	irq_enable();
	return (0);
}
