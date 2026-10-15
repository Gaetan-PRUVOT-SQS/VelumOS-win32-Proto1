#include "fake_sched.h"
#include "velum/irqflags.h"

void	irq_disable(void)
{
	fake_self()->iflag = 0;
}

void	irq_enable(void)
{
	fake_self()->iflag = RFLAGS_IF;
}

void	sched_prio_inherit(t_kthread *owner, int32_t prio)
{
	if (!owner)
		return ;
	fh_lock();
	if (prio > owner->inherit)
		owner->inherit = prio;
	owner->t.prio = policy_prio(owner);
	fh_unlock();
}

void	sched_prio_uninherit(t_kthread *kt)
{
	fh_lock();
	kt->inherit = 0;
	kt->t.prio = policy_prio(kt);
	fh_unlock();
}
