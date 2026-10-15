#include "fake.h"

static uint32_t	chain_write_len(t_vq *q, uint16_t head)
{
	uint32_t	len;
	uint16_t	cur;
	int			guard;

	len = 0;
	cur = head;
	guard = 0;
	while (guard++ < q->size)
	{
		if (q->desc[cur].flags & VQ_DESC_F_WRITE)
			len += q->desc[cur].len;
		if (!(q->desc[cur].flags & VQ_DESC_F_NEXT))
			break ;
		cur = q->desc[cur].next;
	}
	return (len);
}

static void	push(t_vq *q, uint16_t head)
{
	volatile t_vqelem	*e;

	e = &q->used->ring[q->used->idx & (q->size - 1)];
	e->id = head;
	e->len = chain_write_len(q, head);
	q->used->idx++;
}

void	fk_vq_serve(t_vq *q, uint16_t *last, int rev)
{
	uint16_t	n;
	uint16_t	i;
	uint16_t	heads[VQ_SIZE_MAX];

	n = (uint16_t)(q->avail->idx - *last);
	i = 0;
	while (i < n && i < q->size)
	{
		heads[i] = q->avail->ring[(uint16_t)(*last + i) & (q->size - 1)];
		i++;
	}
	i = 0;
	while (i < n && i < q->size)
	{
		if (rev)
			push(q, heads[n - 1 - i]);
		else
			push(q, heads[i]);
		i++;
	}
	*last = q->avail->idx;
}
