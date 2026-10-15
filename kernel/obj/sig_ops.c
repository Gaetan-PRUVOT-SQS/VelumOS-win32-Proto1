#include "obj_int.h"

bool	sig_take_locked(t_object *o)
{
	t_sigstate	*s;

	s = o->impl;
	if (!s->state)
		return (false);
	if (!s->manual)
		__atomic_store_n(&s->state, 0, __ATOMIC_RELEASE);
	return (true);
}

void	sig_set(t_object *o, uint32_t v)
{
	t_sigstate	*s;
	uint64_t	fl;

	s = o->impl;
	fl = sig_lock();
	__atomic_store_n(&s->state, (uint32_t)(v != 0), __ATOMIC_RELEASE);
	sig_unlock(fl);
	if (v)
		obj_signal(o);
}
