#include "velum/arch.h"
#include "velum/display.h"
#include "velum/proc.h"
#include "velum/sched.h"

__attribute__((weak))
void	display_boot_progress(uint32_t pct, const char *stage)
{
	(void)pct;
	(void)stage;
}

__attribute__((weak))
void	display_boot_done(void)
{
}

__attribute__((weak))
int	init_start(void)
{
	return (0);
}

__attribute__((weak))
void	sched_idle(void)
{
	arch_halt_forever();
}

__attribute__((weak))
void	sched_irq_exit(void)
{
}
