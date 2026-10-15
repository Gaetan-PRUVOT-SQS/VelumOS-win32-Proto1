#include "velum/err.h"
#include "velum/klog.h"
#include "velum/util.h"
#include "cpu_int.h"

static int	g_fault = FAULT_NONE;

void	cpu_fault_set(int fault)
{
	g_fault = fault;
}

static int	fault_guarded(int fault)
{
	uint64_t	guard;
	uint64_t	here;

	guard = align_up(cpu_boot_stack_low(), PAGE_SIZE);
	here = (uint64_t)__builtin_frame_address(0);
	if (here < guard + 2 * PAGE_SIZE || cpu_guard_page(guard) < 0)
	{
		klog_err("cpu: page de garde impossible pour fault=%s",
			fault_name(fault));
		return (E_NOTSUP);
	}
	kprintf("fault: page de garde %#llx\n", guard);
	if (fault == FAULT_PF)
		cpu_fault_pf(guard);
	else
		cpu_fault_stack();
	return (E_FAULT);
}

static void	fault_simple(int fault)
{
	if (fault == FAULT_DIV0)
		cpu_fault_div0();
	else if (fault == FAULT_GP)
		cpu_fault_gp();
	else if (fault == FAULT_UD)
		cpu_fault_ud();
	else if (fault == FAULT_DF)
		cpu_fault_df();
	else if (fault == FAULT_NMI)
		cpu_fault_nmi();
}

int	cpu_fault_run(void)
{
	int	fault;

	fault = g_fault;
	if (!CPU_FAULT_INJECT || fault == FAULT_NONE)
		return (E_OK);
	kprintf("\nfault: %s\n", fault_name(fault));
	if (cpu_fault_rip(fault))
		kprintf("fault: rip attendu %#llx\n", cpu_fault_rip(fault));
	if (fault == FAULT_PF || fault == FAULT_STACK)
		return (fault_guarded(fault));
	fault_simple(fault);
	klog_err("cpu: fault=%s n'a pas arrêté le noyau", fault_name(fault));
	return (E_FAULT);
}
