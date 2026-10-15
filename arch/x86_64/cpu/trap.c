#include "velum/klog.h"
#include "velum/panic.h"
#include "cpu_int.h"

static t_trapstate	g_trap;

void	cpu_set_user_fault(t_userfaultfn fn)
{
	__atomic_store_n(&g_trap.hook, fn, __ATOMIC_RELEASE);
}

uint64_t	trap_breakpoints(void)
{
	return (__atomic_load_n(&g_trap.breakpoints, __ATOMIC_RELAXED));
}

static void	trap_user(t_regs *regs, uint64_t cr2)
{
	t_userfaultfn	hook;

	hook = __atomic_load_n(&g_trap.hook, __ATOMIC_ACQUIRE);
	if (!hook)
		trap_kernel_fault(regs, cr2);
	if (regs->vec == VEC_PAGE_FAULT)
		hook(regs, regs->vec, cr2);
	else
		hook(regs, regs->vec, regs->rip);
}

static void	trap_act(t_trapact act, t_regs *regs, const t_trapslot *slot,
				uint64_t cr2)
{
	if (act == TRAP_HANDLER)
		slot->fn(regs, slot->ctx);
	else if (act == TRAP_NESTED)
		panic("faute imbriquée %s (%s), rip %#llx, pendant une faute",
			trap_name(regs->vec), trap_desc(regs->vec), regs->rip);
	else if (act == TRAP_UNHANDLED)
		trap_unhandled(regs);
	else if (act == TRAP_USER_HOOK)
		trap_user(regs, cr2);
	else if (act == TRAP_BREAKPOINT)
	{
		__atomic_add_fetch(&g_trap.breakpoints, 1, __ATOMIC_RELAXED);
		klog_debug("cpu: int3 noyau, rip %#llx", regs->rip);
	}
	else
		trap_kernel_fault(regs, cr2);
}

void	trap_dispatch(t_regs *regs)
{
	t_trapin	in;
	t_trapslot	slot;
	uint32_t	*depth;
	uint64_t	cr2;
	bool		counted;

	cr2 = 0;
	if (regs->vec == VEC_PAGE_FAULT)
		cr2 = cpu_read_cr2();
	depth = &g_trap.depth[0];
	if (cpu_self() && cpu_self()->id < CPU_MAX)
		depth = &g_trap.depth[cpu_self()->id];
	in.vec = regs->vec;
	in.user = (regs->cs & 3) != 0;
	in.nested = *depth > 0;
	in.handler = idt_handler_get((uint8_t)regs->vec, &slot);
	in.hook = __atomic_load_n(&g_trap.hook, __ATOMIC_ACQUIRE) != NULL;
	counted = in.vec < EXC_COUNT && !in.user;
	if (counted)
		(*depth)++;
	trap_act(trap_classify(&in), regs, &slot, cr2);
	if (counted)
		(*depth)--;
	trap_leave_user(in.user);
}
