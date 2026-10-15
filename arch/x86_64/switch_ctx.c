#include "../../kernel/sched/sched_int.h"
#include "velum/msr.h"
#include "velum/vmm.h"

static void	ctx_save(t_kthread *prev, t_cpu *cpu)
{
	prev->irq_depth = cpu->irq_depth;
	prev->user_rsp = cpu->user_rsp;
	if (!(prev->t.flags & THREAD_USER))
		return ;
	prev->t.fs_base = msr_read(MSR_FS_BASE);
	cpu_fpu_save(prev->t.fpu);
}

static void	ctx_load(t_kthread *next, t_cpu *cpu)
{
	cpu->irq_depth = next->irq_depth;
	cpu->user_rsp = next->user_rsp;
	if (!(next->t.flags & THREAD_USER))
		return ;
	msr_write(MSR_FS_BASE, next->t.fs_base);
	cpu_fpu_restore(next->t.fpu);
}

static void	ctx_space(t_cpusched *cs, t_kthread *next)
{
	t_aspace	*as;

	as = next->t.aspace;
	if (!as)
		as = vmm_kernel_aspace();
	if (as == cs->active_as)
		return ;
	vmm_switch(as);
	cs->active_as = as;
}

void	arch_switch_to(t_cpusched *cs, t_kthread *prev, t_kthread *next)
{
	t_cpu	*cpu;

	cpu = cs->cpu;
	ctx_save(prev, cpu);
	cpu->current = &next->t;
	ctx_space(cs, next);
	gdt_set_rsp0(next->t.kstack_top);
	cpu->kstack_top = next->t.kstack_top;
	ctx_load(next, cpu);
	sched_switch_stacks(&prev->t.rsp, next->t.rsp);
}

void	arch_thread_setup(t_kthread *kt)
{
	uint64_t	*frame;

	frame = (uint64_t *)(kt->t.kstack_top - 8 * sizeof(uint64_t));
	frame[0] = 0;
	frame[1] = 0;
	frame[2] = 0;
	frame[3] = (uint64_t)kt;
	frame[4] = 0;
	frame[5] = 0;
	frame[6] = (uint64_t)sched_thread_trampoline;
	frame[7] = 0;
	kt->t.rsp = (uint64_t)frame;
}
