#include "velum/err.h"
#include "virtio.h"

static int	vq_check_bufs(const t_vq *q, const t_vqbuf *b, uint16_t n)
{
	uint16_t	i;
	bool		seen_write;

	if (!b || n == 0 || n > q->size)
		return (E_INVAL);
	i = 0;
	seen_write = false;
	while (i < n)
	{
		if (b[i].len == 0 || b[i].phys == 0 || (seen_write && !b[i].write))
			return (E_INVAL);
		seen_write = b[i].write;
		i++;
	}
	if (n > q->nfree)
		return (E_AGAIN);
	return (0);
}

static void	vq_publish(t_vq *q, uint16_t head)
{
	q->avail->ring[q->avail_idx & (q->size - 1)] = head;
	vio_barrier();
	q->avail_idx++;
	q->avail->idx = q->avail_idx;
	vio_barrier();
}

static uint16_t	vq_fill_chain(t_vq *q, const t_vqbuf *b, uint16_t n)
{
	uint16_t	cur;
	uint16_t	i;

	cur = q->free_head;
	i = 0;
	while (i < n)
	{
		q->desc[cur].addr = b[i].phys;
		q->desc[cur].len = b[i].len;
		q->desc[cur].flags = (uint16_t)(VQ_DESC_F_WRITE * b[i].write
				+ VQ_DESC_F_NEXT * (i + 1 < n));
		q->desc[cur].next = q->next[cur];
		i++;
		if (i < n)
			cur = q->next[cur];
	}
	return (cur);
}

int	vq_add(t_vq *q, const t_vqbuf *b, uint16_t n)
{
	uint16_t	head;
	uint16_t	last;
	int			rc;

	rc = vq_check_bufs(q, b, n);
	if (rc < 0)
		return (rc);
	head = q->free_head;
	last = vq_fill_chain(q, b, n);
	q->free_head = q->next[last];
	q->nfree -= n;
	q->chain[head] = n;
	q->inflight++;
	vq_publish(q, head);
	return (head);
}
