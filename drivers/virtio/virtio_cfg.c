#include "velum/err.h"
#include "velum/libk.h"
#include "velum/pmm.h"
#include "virtio.h"

static void	cfg_raw(t_vio *v, uint32_t off, void *out, uint32_t len)
{
	uint64_t	val;

	if (len == 1)
		val = vio_rd8(v->devcfg, off);
	else if (len == 2)
		val = vio_rd16(v->devcfg, off);
	else if (len == 4)
		val = vio_rd32(v->devcfg, off);
	else
		val = (uint64_t)vio_rd32(v->devcfg, off)
			| ((uint64_t)vio_rd32(v->devcfg, off + 4) << 32);
	memcpy(out, &val, len);
}

int	vio_cfg_read(t_vio *v, uint32_t off, void *out, uint32_t len)
{
	uint8_t	gen;
	int		tries;

	if (!v->devcfg)
		return (E_NODEV);
	if (len != 1 && len != 2 && len != 4 && len != 8)
		return (E_INVAL);
	if ((off & (len - 1)) || (uint64_t)off + len > v->devcfg_len)
		return (E_RANGE);
	tries = 0;
	while (tries < VIO_GEN_TRIES)
	{
		gen = vio_rd8(v->common, VIO_CC_GENERATION);
		cfg_raw(v, off, out, len);
		if (vio_rd8(v->common, VIO_CC_GENERATION) == gen)
			return (0);
		tries++;
	}
	return (E_IO);
}

void	vio_queue_release(t_vq *q)
{
	if (q->phys)
		pmm_free_pages(q->phys, q->pages, PMM_DMA);
	q->phys = 0;
	q->pages = 0;
}

void	vio_kick(t_vq *q)
{
	vio_barrier();
	vio_wr16(q->notify, 0, q->index);
}
