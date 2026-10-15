#include "velum/pmm.h"
#include "velum/vmm.h"
#include "vblk.h"

#define VBLK_WANTED 0x200000262ull

static int	vblk_hw_init(t_vblk *v, const t_pcidev *d)
{
	int	rc;

	rc = vio_find_caps(d, &v->vio);
	if (rc == 0 && pci_enable(d, PCI_CMD_MEM | PCI_CMD_MASTER) < 0)
		rc = E_IO;
	if (rc == 0)
		vblk_intx_off(d);
	if (rc == 0)
		rc = vio_map(&v->vio);
	if (rc == 0)
		rc = vio_reset(&v->vio);
	if (rc == 0)
		rc = vio_negotiate(&v->vio, VBLK_WANTED);
	if (rc == 0)
		rc = vblk_geometry(v);
	return (rc);
}

static int	vblk_dma_init(t_vblk *v)
{
	int	rc;

	rc = vio_queue_setup(&v->vio, 0, &v->q);
	if (rc < 0)
		return (rc);
	if (v->q.size < VBLK_QUEUE_MIN)
		return (E_NOTSUP);
	v->dma_phys = pmm_alloc_pages(PMM_DMA, VBLK_DMA_PAGES, 1, 0);
	if (!v->dma_phys)
		return (E_NOMEM);
	v->dma = phys_to_virt(v->dma_phys);
	memset(v->dma, 0, VBLK_BOUNCE_OFF);
	return (vio_start(&v->vio));
}

static void	vblk_destroy(t_vblk *v, int rc)
{
	klog_warn("block: %s: initialisation refusée (%d)", v->dev.name, rc);
	if (v->q.phys || v->dma_phys)
	{
		if (vio_reset(&v->vio) < 0)
		{
			klog_err("block: %s: remise à zéro impossible, DMA conservée",
				v->dev.name);
			return ;
		}
		vio_queue_release(&v->q);
		if (v->dma_phys)
			pmm_free_pages(v->dma_phys, VBLK_DMA_PAGES, PMM_DMA);
	}
	else if (v->vio.common)
		vio_status_add(&v->vio, VIO_ST_FAILED);
	kfree(v);
}

static void	vblk_announce(const t_vblk *v)
{
	const char	*ro;

	ro = "";
	if (v->dev.flags & BLK_RO)
		ro = ", lecture seule";
	klog_info("block: %s virtio-blk, %llu secteurs de %u octets%s",
		v->dev.name, (unsigned long long)v->dev.nsectors,
		v->dev.sector_size, ro);
}

int	vblk_probe(const t_pcidev *d, uint32_t idx)
{
	t_vblk	*v;
	int		rc;

	if (idx >= VBLK_MAX_DISKS)
		return (E_NOSPC);
	v = kmalloc_tag(sizeof(*v), HEAP_BLOCK);
	if (!v)
		return (E_NOMEM);
	memset(v, 0, sizeof(*v));
	mutex_init(&v->lock, "vblk.req");
	memcpy(v->dev.name, "vd", 2);
	v->dev.name[2] = (char)('a' + idx);
	v->dev.ops = vblk_ops();
	v->dev.priv = v;
	rc = vblk_hw_init(v, d);
	if (rc == 0)
		rc = vblk_dma_init(v);
	if (rc == 0)
		rc = blk_register(&v->dev);
	if (rc < 0)
		vblk_destroy(v, rc);
	else
		vblk_announce(v);
	return (rc);
}
