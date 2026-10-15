#include "velum/err.h"
#include "velum/libk.h"
#include "velum/pmm.h"
#include "velum/util.h"
#include "velum/vmm.h"
#include "virtio.h"

static uint16_t	pow2_floor(uint16_t n)
{
	uint16_t	p;

	p = 1;
	while (p * 2 <= n && p * 2 <= VQ_SIZE_MAX)
		p *= 2;
	return (p);
}

static int	queue_alloc(t_vq *q, uint16_t qsz)
{
	q->pages = (vq_mem_size(qsz) + PAGE_SIZE - 1) / PAGE_SIZE;
	q->phys = pmm_alloc_pages(PMM_DMA, q->pages, 1, 0);
	if (!q->phys)
	{
		q->pages = 0;
		return (E_NOMEM);
	}
	return (vq_init(q, phys_to_virt(q->phys), q->phys, qsz));
}

static int	queue_notify_addr(t_vio *v, t_vq *q)
{
	uint64_t	off;

	off = (uint64_t)vio_rd16(v->common, VIO_CC_QNOTIFY_OFF) * v->notify_mult;
	if (off > v->notify_len || v->notify_len - off < 2)
		return (E_IO);
	q->notify = v->notify + off;
	return (0);
}

static void	queue_program(t_vio *v, const t_vq *q)
{
	uintptr_t	base;

	base = (uintptr_t)q->desc;
	vio_wr16(v->common, VIO_CC_QSIZE, q->size);
	vio_wr16(v->common, VIO_CC_QMSIX, VIO_NO_VECTOR);
	vio_wr64(v->common, VIO_CC_QDESC, q->phys);
	vio_wr64(v->common, VIO_CC_QDRIVER, q->phys + ((uintptr_t)q->avail - base));
	vio_wr64(v->common, VIO_CC_QDEVICE, q->phys + ((uintptr_t)q->used - base));
	vio_wr16(v->common, VIO_CC_QENABLE, 1);
}

int	vio_queue_setup(t_vio *v, uint16_t index, t_vq *q)
{
	uint16_t	dev_size;
	int			rc;

	memset(q, 0, sizeof(*q));
	q->index = index;
	if (index >= vio_rd16(v->common, VIO_CC_NUMQ))
		return (E_NODEV);
	vio_wr16(v->common, VIO_CC_QSELECT, index);
	dev_size = vio_rd16(v->common, VIO_CC_QSIZE);
	if (dev_size == 0)
		return (E_NODEV);
	rc = queue_alloc(q, pow2_floor(dev_size));
	if (rc == 0)
		rc = queue_notify_addr(v, q);
	if (rc < 0)
	{
		vio_queue_release(q);
		return (rc);
	}
	queue_program(v, q);
	return (0);
}
