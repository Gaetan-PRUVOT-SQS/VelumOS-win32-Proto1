#include "sched_int.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/irq.h"
#include "velum/klog.h"
#include "velum/libk.h"

void	sched_kick(t_cpusched *cs)
{
	if (cs->cpu && cs->cpu != cpu_self())
	{
		__atomic_fetch_add(&cs->kicks, 1, __ATOMIC_RELAXED);
		sched_smp_kick(cs->id);
	}
}

void	sched_kick_idle(t_cpusched *cs)
{
	uint32_t	peer;

	peer = sched_idle_peer(g_sched.cpus, g_sched.span, cs->id);
	if (peer < g_sched.span)
		sched_kick(g_sched.cpus[peer]);
}

void	sched_ipi_resched(t_regs *regs, void *ctx)
{
	t_cpusched	*cs;

	(void)regs;
	(void)ctx;
	cs = cpu_self()->sched;
	if (cs)
		__atomic_store_n(&cs->need_resched, 1, __ATOMIC_RELEASE);
	apic_eoi();
	sched_irq_exit();
}

void	sched_smp_setup(void)
{
	if (idt_set_handler(VEC_RESCHED, sched_ipi_resched, NULL) < 0)
		klog_warn("sched: vecteur de reschedule 0x%x indisponible",
			VEC_RESCHED);
}

static void	cpu_publish(t_cpusched *cs)
{
	__atomic_store_n(&g_sched.cpus[cs->id], cs, __ATOMIC_RELEASE);
	if (cs->id + 1 > __atomic_load_n(&g_sched.span, __ATOMIC_RELAXED))
		__atomic_store_n(&g_sched.span, cs->id + 1, __ATOMIC_RELEASE);
}

static t_kthread	*cpu_boot_thread(t_cpu *cpu)
{
	t_kthread	*kt;

	kt = kmalloc_tag(sizeof(*kt), HEAP_SCHED);
	if (!kt)
		return (NULL);
	memset(kt, 0, sizeof(*kt));
	kt->t.tid = __atomic_fetch_add(&g_sched.next_tid, 1, __ATOMIC_RELAXED);
	strlcpy(kt->t.name, "idle", sizeof(kt->t.name));
	kt->t.state = TS_RUNNING;
	kt->t.prio = PRIO_IDLE;
	kt->t.refs = 1;
	kt->t.quantum_left = QUANTUM_NS;
	kt->t.kstack_top = cpu->kstack_top;
	kt->kflags = KT_STATIC | KT_IDLE | KT_PINNED;
	kt->home = cpu->id;
	waitq_init(&kt->t.joiners);
	return (kt);
}

static int	cpu_reaper(t_cpusched *cs)
{
	t_threadreq	rq;

	memset(&rq, 0, sizeof(rq));
	rq.name = "reaper";
	rq.fn = sched_reaper_main;
	rq.arg = cs;
	rq.prio = PRIO_HIGH;
	rq.flags = THREAD_DETACHED;
	cs->reaper = thread_alloc(&rq);
	if (!cs->reaper)
		return (E_NOMEM);
	cs->reaper->kflags |= KT_PINNED;
	cs->reaper->home = cs->id;
	return (0);
}

int	sched_cpu_online(t_cpu *cpu)
{
	t_cpusched	*cs;
	t_kthread	*idle;

	if (!cpu || cpu->id == 0 || cpu->id >= CPU_MAX
		|| !__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE)
		|| g_sched.cpus[cpu->id])
		return (E_INVAL);
	cs = kmalloc_tag(sizeof(*cs), HEAP_SCHED);
	idle = cpu_boot_thread(cpu);
	if (!cs || !idle)
	{
		kfree(cs);
		kfree(idle);
		return (E_NOMEM);
	}
	memset(cs, 0, sizeof(*cs));
	sched_cpusched_init(cs, cpu);
	cs->idle = idle;
	cpu->sched = cs;
	cpu->current = &idle->t;
	cpu->preempt = 0;
	if (cpu_reaper(cs) < 0)
	{
		cpu->sched = NULL;
		cpu->current = NULL;
		kfree(cs);
		kfree(idle);
		return (E_NOMEM);
	}
	cpu_publish(cs);
	sched_unpark(cs->reaper, false);
	klog_info("sched: cœur %u en ligne", cpu->id);
	return (0);
}
