#include <stdint.h>
#include "velum/irqflags.h"

uint64_t	irq_save(void)
{
	return (0x200);
}

void	irq_restore(uint64_t flags)
{
	(void)flags;
}

void	cpu_relax(void)
{
}
