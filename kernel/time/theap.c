#include "time_int.h"
#include "velum/err.h"

void	theap_init(t_theap *h, t_tslot *slots, uint32_t *idx, uint32_t cap)
{
	uint32_t	k;

	h->slot = slots;
	h->heap = idx;
	h->freestk = idx + cap;
	h->cap = cap;
	h->n = 0;
	h->seq = 0;
	h->reserved = 0;
	k = 0;
	while (k < cap)
	{
		slots[k].pos = -1;
		slots[k].gen = 0;
		h->freestk[k] = cap - 1 - k;
		k++;
	}
	h->nfree = cap;
}

int64_t	theap_insert(t_theap *h, uint64_t deadline, t_timerfn fn, void *ctx)
{
	uint32_t	s;
	t_tslot		*t;

	if (!h->nfree)
		return (E_NOMEM);
	h->nfree--;
	s = h->freestk[h->nfree];
	t = &h->slot[s];
	t->deadline = deadline;
	t->seq = h->seq;
	h->seq++;
	t->fn = fn;
	t->ctx = ctx;
	t->gen = (t->gen + 1) & TIMER_GEN_MASK;
	if (!t->gen)
		t->gen = 1;
	t->pos = (int32_t)h->n;
	h->heap[h->n] = s;
	h->n++;
	theap_sift_up(h, (uint32_t)t->pos);
	return (((int64_t)t->gen << 32) | (int64_t)(s + 1));
}

bool	theap_cancel(t_theap *h, int64_t id)
{
	uint32_t	s;
	uint32_t	gen;

	if (id <= 0)
		return (false);
	s = (uint32_t)(id & 0xffffffff) - 1;
	gen = (uint32_t)(id >> 32);
	if (s >= h->cap || h->slot[s].pos < 0 || h->slot[s].gen != gen)
		return (false);
	theap_remove_at(h, (uint32_t)h->slot[s].pos);
	return (true);
}

uint64_t	theap_peek(const t_theap *h)
{
	if (!h->n)
		return (UINT64_MAX);
	return (h->slot[h->heap[0]].deadline);
}

bool	theap_pop(t_theap *h, uint64_t now, t_tslot *out)
{
	if (!h->n || h->slot[h->heap[0]].deadline > now)
		return (false);
	*out = h->slot[h->heap[0]];
	theap_remove_at(h, 0);
	return (true);
}
