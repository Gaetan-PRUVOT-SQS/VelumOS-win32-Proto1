#include "obj_int.h"

static t_handle	ht_commit_one(t_htab *ht, uint32_t idx, t_object *o,
		uint32_t rt)
{
	uint64_t	fl;
	t_handle	h;

	h = HANDLE_INVALID;
	fl = spin_lock_irqsave(&ht->lock);
	if (ht->closed)
		ht_free_slot(ht, idx);
	else
	{
		ht->ents[idx].obj = o;
		ht->ents[idx].rights = rt;
		ht->ents[idx].state = HE_USED;
		h = ht_encode(ht, idx);
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	return (h);
}

int	handle_alloc(t_process *p, t_object *o, uint32_t rt, t_handle *out)
{
	t_htab	*ht;
	int		idx;

	ht = ht_of(p);
	if (!ht || !o || !out || (rt & ~HR_ALL))
		return (E_INVAL);
	idx = ht_reserve_one(ht);
	if (idx < 0)
		return (idx);
	obj_ref(o);
	*out = ht_commit_one(ht, (uint32_t)idx, o, rt);
	if (*out != HANDLE_INVALID)
		return (0);
	obj_unref(o);
	return (E_CANCELED);
}

int	handle_get(t_process *p, t_handle h, uint32_t type, t_hget *out)
{
	t_htab		*ht;
	t_hentry	*e;
	uint64_t	fl;
	int			rc;

	ht = ht_of(p);
	if (!ht || !out)
		return (E_BADF);
	fl = spin_lock_irqsave(&ht->lock);
	e = ht_lookup(ht, h);
	rc = E_BADF;
	if (e && e->state == HE_BUSY)
		rc = E_BUSY;
	else if (e && (type == OBJ_NONE || e->obj->type == type))
	{
		obj_ref(e->obj);
		out->obj = e->obj;
		out->rights = e->rights;
		rc = 0;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	return (rc);
}

int	handle_need(t_hget *g, uint32_t need)
{
	if ((g->rights & need) == need)
		return (0);
	obj_unref(g->obj);
	g->obj = NULL;
	return (E_PERM);
}

int	handle_close(t_process *p, t_handle h)
{
	t_htab		*ht;
	t_hentry	*e;
	t_object	*o;
	uint64_t	fl;
	int			rc;

	ht = ht_of(p);
	if (!ht)
		return (E_BADF);
	o = NULL;
	fl = spin_lock_irqsave(&ht->lock);
	e = ht_lookup(ht, h);
	rc = E_BADF;
	if (e && e->state == HE_BUSY)
		rc = E_BUSY;
	else if (e)
	{
		o = e->obj;
		ht_free_slot(ht, h & HT_IDX_MASK);
		rc = 0;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	obj_unref(o);
	return (rc);
}
