#include "obj_int.h"

static t_spinlock	g_sig_lock;

void	sig_init(void)
{
	spin_init(&g_sig_lock, "sig");
}

uint64_t	sig_lock(void)
{
	return (spin_lock_irqsave(&g_sig_lock));
}

void	sig_unlock(uint64_t flags)
{
	spin_unlock_irqrestore(&g_sig_lock, flags);
}

bool	sig_signaled(t_object *o)
{
	t_sigstate	*s;

	s = o->impl;
	return (__atomic_load_n(&s->state, __ATOMIC_ACQUIRE) != 0);
}

bool	sig_is(t_object *o)
{
	return (o->ops->signaled == sig_signaled);
}
