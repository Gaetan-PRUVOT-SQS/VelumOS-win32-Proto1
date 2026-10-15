#include "obj_int.h"
#include "velum/heap.h"
#include "velum/util.h"

static bool	overlaps(const t_secmap *m, uintptr_t va, uint64_t len)
{
	if (m->va >= va)
		return (m->va - va < len);
	return (va - m->va < m->len);
}

static t_secmap	*take_overlapping(t_htab *ht, uintptr_t va, uint64_t len)
{
	uint64_t	fl;
	t_secmap	**link;
	t_secmap	*m;
	t_secmap	*taken;

	taken = NULL;
	fl = spin_lock_irqsave(&ht->lock);
	link = &ht->maps;
	while (*link)
	{
		m = *link;
		if (overlaps(m, va, len))
		{
			*link = m->next;
			m->next = taken;
			taken = m;
		}
		else
			link = &m->next;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	return (taken);
}

static bool	settle(t_htab *ht, t_secmap *m, bool alive)
{
	uint64_t	fl;
	bool		kept;

	fl = spin_lock_irqsave(&ht->lock);
	kept = alive && !ht->closed;
	if (kept)
	{
		m->next = ht->maps;
		ht->maps = m;
	}
	else if (ht->nmaps > 0)
		ht->nmaps--;
	spin_unlock_irqrestore(&ht->lock, fl);
	return (kept);
}

void	secmaps_forget(t_process *p, uintptr_t va, uint64_t len)
{
	t_htab		*ht;
	t_secmap	*m;
	t_secmap	*next;

	if (!p || !ht_of(p) || !p->aspace || len == 0)
		return ;
	ht = ht_of(p);
	mutex_lock(&ht->maplock);
	m = take_overlapping(ht, va, len);
	while (m)
	{
		next = m->next;
		if (!settle(ht, m, sec_pages_mapped(p->aspace, m)))
			secmap_drop(p->aspace, m);
		m = next;
	}
	mutex_unlock(&ht->maplock);
}
