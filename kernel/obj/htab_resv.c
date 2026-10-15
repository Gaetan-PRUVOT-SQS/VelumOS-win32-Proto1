#include "obj_int.h"

static void	unreserve_n(t_htab *ht, t_xfer *x, uint32_t n)
{
	uint64_t	fl;
	uint32_t	i;

	fl = spin_lock_irqsave(&ht->lock);
	i = 0;
	while (i < n)
	{
		if (ht->ents[x->idx[i]].state == HE_RESERVED)
			ht_free_slot(ht, x->idx[i]);
		i++;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
}

int	htab_reserve(t_htab *ht, t_xfer *x)
{
	uint32_t	i;
	uint64_t	fl;
	int			idx;

	if (!ht || x->n > IPC_HANDLES_MAX)
		return (E_BADF);
	i = 0;
	while (i < x->n)
	{
		idx = ht_reserve_one(ht);
		if (idx < 0)
		{
			unreserve_n(ht, x, i);
			return (idx);
		}
		x->idx[i] = (uint32_t)idx;
		fl = spin_lock_irqsave(&ht->lock);
		x->hs[i] = ht_encode(ht, (uint32_t)idx);
		spin_unlock_irqrestore(&ht->lock, fl);
		i++;
	}
	return (0);
}

void	htab_unreserve(t_htab *ht, t_xfer *x)
{
	unreserve_n(ht, x, x->n);
}

static bool	commit_all(t_htab *ht, t_xfer *x)
{
	uint32_t	i;
	t_hentry	*e;

	i = 0;
	while (i < x->n)
	{
		e = &ht->ents[x->idx[i]];
		if (ht->closed || e->state != HE_RESERVED)
			return (false);
		e->obj = x->objs[i];
		e->rights = x->rights[i];
		e->state = HE_USED;
		x->objs[i] = NULL;
		i++;
	}
	return (true);
}

int	htab_commit(t_htab *ht, t_xfer *x)
{
	uint64_t	fl;
	bool		ok;

	fl = spin_lock_irqsave(&ht->lock);
	ok = commit_all(ht, x);
	spin_unlock_irqrestore(&ht->lock, fl);
	if (ok)
		return (0);
	unreserve_n(ht, x, x->n);
	xfer_drop(x);
	return (E_CANCELED);
}
