#include "obj_int.h"

static t_object	*close_next(t_htab *ht, uint32_t *i)
{
	uint64_t	fl;
	t_object	*o;
	t_hentry	*e;

	o = NULL;
	fl = spin_lock_irqsave(&ht->lock);
	ht->closed = 1;
	while (*i < ht->cap && !o)
	{
		e = &ht->ents[*i];
		if (e->state == HE_USED || e->state == HE_BUSY)
		{
			o = e->obj;
			ht_free_slot(ht, *i);
		}
		(*i)++;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	return (o);
}

void	htab_close_all(t_htab *ht)
{
	uint32_t	i;
	t_object	*o;

	if (!ht)
		return ;
	i = 0;
	o = close_next(ht, &i);
	while (o)
	{
		obj_unref(o);
		o = close_next(ht, &i);
	}
}
