#include "velum/err.h"
#include "velum/libk.h"
#include "virtio.h"

static size_t	vq_used_off(uint16_t qsz)
{
	return ((16 * (size_t)qsz + 6 + 2 * (size_t)qsz + 3) & ~(size_t)3);
}

size_t	vq_mem_size(uint16_t qsz)
{
	return (vq_used_off(qsz) + 6 + 8 * (size_t)qsz);
}

static void	vq_reset_state(t_vq *q, uint16_t qsz)
{
	uint16_t	i;

	q->size = qsz;
	q->free_head = 0;
	q->nfree = qsz;
	q->inflight = 0;
	q->avail_idx = 0;
	q->last_used = 0;
	i = 0;
	while (i < qsz)
	{
		q->next[i] = i + 1;
		q->chain[i] = 0;
		i++;
	}
}

int	vq_init(t_vq *q, void *mem, uint64_t phys, uint16_t qsz)
{
	uint8_t	*base;

	if (!mem || qsz == 0 || qsz > VQ_SIZE_MAX || (qsz & (qsz - 1)))
		return (E_INVAL);
	base = mem;
	memset(base, 0, vq_mem_size(qsz));
	q->desc = (volatile t_vqdesc *)base;
	q->avail = (volatile t_vqavail *)(base + 16 * (size_t)qsz);
	q->used = (volatile t_vqused *)(base + vq_used_off(qsz));
	q->phys = phys;
	vq_reset_state(q, qsz);
	q->avail->flags = VQ_AVAIL_F_NO_INTERRUPT;
	return (0);
}
