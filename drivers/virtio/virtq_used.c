#include "velum/err.h"
#include "virtio.h"

void	vio_barrier(void)
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

bool	vq_used_pending(void *ctx)
{
	t_vq	*q;

	q = ctx;
	return (q->used->idx != q->last_used);
}

static void	vq_free_chain(t_vq *q, uint16_t head)
{
	uint16_t	left;
	uint16_t	last;

	left = q->chain[head];
	last = head;
	while (left > 1)
	{
		last = q->next[last];
		left--;
	}
	q->next[last] = q->free_head;
	q->free_head = head;
	q->nfree += q->chain[head];
	q->chain[head] = 0;
	q->inflight--;
}

int	vq_get(t_vq *q, uint32_t *len)
{
	uint16_t			uidx;
	uint32_t			id;
	volatile t_vqelem	*e;

	uidx = q->used->idx;
	vio_barrier();
	if (uidx == q->last_used)
		return (E_AGAIN);
	if ((uint16_t)(uidx - q->last_used) > q->inflight)
		return (E_IO);
	e = &q->used->ring[q->last_used & (q->size - 1)];
	id = e->id;
	if (id >= q->size || q->chain[id] == 0)
		return (E_IO);
	if (len)
		*len = e->len;
	vq_free_chain(q, (uint16_t)id);
	q->last_used++;
	return ((int)id);
}
