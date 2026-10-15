#include "fakes.h"
#include "velum/arch.h"
#include "velum/irqflags.h"

uint64_t	irq_save(void)
{
	g_fk.irq_depth++;
	return (0);
}

void	irq_restore(uint64_t flags)
{
	(void)flags;
	g_fk.irq_depth--;
}

void	cpu_relax(void)
{
}

_Noreturn void	arch_halt_forever(void)
{
	g_fk.halts++;
	longjmp(g_fk.halt_env, 1);
}
