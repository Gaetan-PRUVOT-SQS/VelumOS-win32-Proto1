#include "obj_int.h"

static int	take_check(t_htab *ht, t_xfer *x)
{
	uint32_t	i;
	uint32_t	j;
	t_hentry	*e;

	i = 0;
	while (i < x->n)
	{
		e = ht_lookup(ht, x->hs[i]);
		if (!e || e->state != HE_USED)
			return (E_BADF);
		if (!(e->rights & HR_TRANSFER))
			return (E_PERM);
		j = 0;
		while (j < i)
		{
			if (x->hs[j] == x->hs[i])
				return (E_INVAL);
			j++;
		}
		i++;
	}
	return (0);
}

static void	take_apply(t_htab *ht, t_xfer *x)
{
	uint32_t	i;
	t_hentry	*e;

	i = 0;
	while (i < x->n)
	{
		e = ht_lookup(ht, x->hs[i]);
		x->idx[i] = x->hs[i] & HT_IDX_MASK;
		x->objs[i] = e->obj;
		x->rights[i] = e->rights;
		obj_ref(e->obj);
		if (x->move)
			e->state = HE_BUSY;
		i++;
	}
}

int	htab_take(t_htab *ht, t_xfer *x)
{
	uint64_t	fl;
	int			rc;

	if (!ht || x->n > IPC_HANDLES_MAX)
		return (E_BADF);
	fl = spin_lock_irqsave(&ht->lock);
	rc = take_check(ht, x);
	if (rc == 0)
		take_apply(ht, x);
	spin_unlock_irqrestore(&ht->lock, fl);
	return (rc);
}

void	htab_take_end(t_htab *ht, t_xfer *x, bool commit)
{
	t_object	*drop[IPC_HANDLES_MAX];
	t_hentry	*e;
	uint32_t	i;
	uint32_t	nd;
	uint64_t	fl;

	nd = 0;
	i = 0;
	fl = spin_lock_irqsave(&ht->lock);
	while (x->move && i < x->n)
	{
		e = &ht->ents[x->idx[i]];
		if (e->state == HE_BUSY && commit)
		{
			drop[nd] = e->obj;
			nd++;
			ht_free_slot(ht, x->idx[i]);
		}
		else if (e->state == HE_BUSY)
			e->state = HE_USED;
		i++;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	while (nd > 0)
		obj_unref(drop[--nd]);
}

void	xfer_drop(t_xfer *x)
{
	while (x->n > 0)
	{
		x->n--;
		obj_unref(x->objs[x->n]);
		x->objs[x->n] = NULL;
	}
}
