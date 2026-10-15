#include "obj_int.h"

t_htab	*ht_of(t_process *p)
{
	if (!p)
		return (NULL);
	return (p->handles);
}

int	ht_find_free(t_htab *ht)
{
	uint32_t	i;
	t_hentry	*e;

	i = ht->hint;
	while (i < ht->cap)
	{
		e = &ht->ents[i];
		if (e->state == HE_FREE)
		{
			e->state = HE_RESERVED;
			if (e->gen == 0)
				e->gen = 1;
			ht->used++;
			ht->hint = i + 1;
			return ((int)i);
		}
		i++;
	}
	ht->hint = ht->cap;
	return (E_NOSPC);
}

t_hentry	*ht_lookup(t_htab *ht, t_handle h)
{
	uint32_t	idx;
	uint32_t	mask;
	t_hentry	*e;

	idx = h & HT_IDX_MASK;
	if (h == HANDLE_INVALID || idx >= ht->cap)
		return (NULL);
	mask = 0u - (uint32_t)(idx < ht->cap);
	e = &ht->ents[idx & mask];
	if (e->gen != (h >> HT_IDX_BITS))
		return (NULL);
	if (e->state != HE_USED && e->state != HE_BUSY)
		return (NULL);
	return (e);
}

t_handle	ht_encode(t_htab *ht, uint32_t idx)
{
	return ((ht->ents[idx].gen << HT_IDX_BITS) | idx);
}

void	ht_free_slot(t_htab *ht, uint32_t idx)
{
	t_hentry	*e;

	e = &ht->ents[idx];
	e->obj = NULL;
	e->rights = 0;
	e->gen++;
	e->state = HE_FREE;
	if (e->gen > HT_GEN_MAX)
		e->state = HE_RETIRED;
	ht->used--;
	if (idx < ht->hint)
		ht->hint = idx;
}
