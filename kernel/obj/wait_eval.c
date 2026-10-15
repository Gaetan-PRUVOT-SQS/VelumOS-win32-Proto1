#include "obj_int.h"

static bool	link_pulsed(t_wlink *l, bool take)
{
	if (take)
		return (__atomic_exchange_n(&l->pulsed, 0, __ATOMIC_ACQ_REL) != 0);
	return (__atomic_load_n(&l->pulsed, __ATOMIC_ACQUIRE) != 0);
}

static bool	ready_locked(t_object *o, t_wlink *l)
{
	if (link_pulsed(l, false))
		return (true);
	if (sig_is(o))
		return (((t_sigstate *)o->impl)->state != 0);
	return (o->ops->signaled && o->ops->signaled(o));
}

static int	try_any(t_waitset *ws)
{
	uint32_t	i;
	uint64_t	fl;
	bool		ok;
	t_object	*o;

	i = 0;
	while (i < ws->n)
	{
		o = ws->objs[i];
		ok = link_pulsed(&ws->links[i], true);
		if (!ok && sig_is(o))
		{
			fl = sig_lock();
			ok = sig_take_locked(o);
			sig_unlock(fl);
		}
		else if (!ok && o->ops->signaled)
			ok = o->ops->signaled(o);
		if (ok)
			return ((int)i);
		i++;
	}
	return (E_AGAIN);
}

static int	try_all(t_waitset *ws)
{
	uint32_t	i;
	uint64_t	fl;
	bool		ok;

	fl = sig_lock();
	ok = true;
	i = 0;
	while (ok && i < ws->n)
	{
		ok = ready_locked(ws->objs[i], &ws->links[i]);
		i++;
	}
	i = 0;
	while (ok && i < ws->n)
	{
		link_pulsed(&ws->links[i], true);
		if (sig_is(ws->objs[i]))
			sig_take_locked(ws->objs[i]);
		i++;
	}
	sig_unlock(fl);
	if (ok)
		return (0);
	return (E_AGAIN);
}

int	wset_try(t_waitset *ws)
{
	if (ws->mode == WAIT_ALL)
		return (try_all(ws));
	return (try_any(ws));
}
