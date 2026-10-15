#include "time_int.h"

static bool	before(const t_theap *h, uint32_t a, uint32_t b)
{
	const t_tslot	*x;
	const t_tslot	*y;

	x = &h->slot[h->heap[a]];
	y = &h->slot[h->heap[b]];
	return (x->deadline < y->deadline
		|| (x->deadline == y->deadline && x->seq < y->seq));
}

static void	swap_pos(t_theap *h, uint32_t a, uint32_t b)
{
	uint32_t	tmp;

	tmp = h->heap[a];
	h->heap[a] = h->heap[b];
	h->heap[b] = tmp;
	h->slot[h->heap[a]].pos = (int32_t)a;
	h->slot[h->heap[b]].pos = (int32_t)b;
}

void	theap_sift_up(t_theap *h, uint32_t pos)
{
	uint32_t	parent;

	while (pos > 0)
	{
		parent = (pos - 1) / 2;
		if (!before(h, pos, parent))
			return ;
		swap_pos(h, pos, parent);
		pos = parent;
	}
}

void	theap_sift_down(t_theap *h, uint32_t pos)
{
	uint32_t	l;
	uint32_t	m;

	while (1)
	{
		l = 2 * pos + 1;
		m = pos;
		if (l < h->n && before(h, l, m))
			m = l;
		if (l + 1 < h->n && before(h, l + 1, m))
			m = l + 1;
		if (m == pos)
			return ;
		swap_pos(h, pos, m);
		pos = m;
	}
}

void	theap_remove_at(t_theap *h, uint32_t pos)
{
	uint32_t	s;

	s = h->heap[pos];
	h->n--;
	if (pos != h->n)
	{
		h->heap[pos] = h->heap[h->n];
		h->slot[h->heap[pos]].pos = (int32_t)pos;
		theap_sift_down(h, pos);
		theap_sift_up(h, pos);
	}
	h->slot[s].pos = -1;
	h->freestk[h->nfree] = s;
	h->nfree++;
}
