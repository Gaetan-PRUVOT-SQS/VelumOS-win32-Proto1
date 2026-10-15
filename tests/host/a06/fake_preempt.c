#include "fake_sched.h"
#include "velum/irqflags.h"
#include "velum/panic.h"

void	sched_preempt_disable(void)
{
	fake_self()->preempt++;
}

void	sched_preempt_enable(void)
{
	t_fakethr	*f;

	f = fake_self();
	if (f->preempt == 0)
		panic("preempt: compteur négatif (hôte)");
	f->preempt--;
}

uint64_t	irq_save(void)
{
	t_fakethr	*f;
	uint64_t	old;

	f = fake_self();
	old = f->iflag;
	f->iflag = 0;
	return (old);
}

void	irq_restore(uint64_t flags)
{
	fake_self()->iflag = flags & RFLAGS_IF;
}

void	cpu_relax(void)
{
	t_fakethr	*f;

	f = fake_self();
	f->relax++;
	if ((f->relax & 63) == 0)
		fh_yield();
	else
		__builtin_ia32_pause();
}
