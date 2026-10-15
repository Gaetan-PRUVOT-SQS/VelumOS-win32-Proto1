#include "cpu_int.h"

t_trapact	trap_classify(const t_trapin *in)
{
	bool	exception;

	exception = in->vec < EXC_COUNT;
	if (exception && !in->user && in->nested)
		return (TRAP_NESTED);
	if (in->handler)
		return (TRAP_HANDLER);
	if (!exception)
		return (TRAP_UNHANDLED);
	if (in->user && in->hook)
		return (TRAP_USER_HOOK);
	if (in->user)
		return (TRAP_USER_PANIC);
	if (in->vec == 3)
		return (TRAP_BREAKPOINT);
	return (TRAP_KERNEL_PANIC);
}

uint8_t	idt_vec_ist(uint32_t vec)
{
	if (vec == 8)
		return (IST_DOUBLE_FAULT);
	if (vec == 2)
		return (IST_NMI);
	if (vec == 18)
		return (IST_MCE);
	return (0);
}

uint8_t	idt_vec_dpl(uint32_t vec)
{
	if (vec == 3)
		return (3);
	return (0);
}

uint64_t	cr0_wanted(uint64_t cr0)
{
	cr0 |= CR0_WP | CR0_NE | CR0_MP;
	cr0 &= ~(CR0_EM | CR0_TS);
	return (cr0);
}

uint64_t	cr4_wanted(uint64_t cr4, const t_cpufeat *feat)
{
	cr4 |= CR4_OSFXSR | CR4_OSXMMEXCPT;
	if (feat->xsave)
		cr4 |= CR4_OSXSAVE;
	if (feat->pge)
		cr4 |= CR4_PGE;
	if (feat->smep)
		cr4 |= CR4_SMEP;
	if (feat->smap)
		cr4 |= CR4_SMAP;
	if (feat->umip)
		cr4 |= CR4_UMIP;
	if (feat->fsgsbase)
		cr4 |= CR4_FSGSBASE;
	return (cr4);
}
