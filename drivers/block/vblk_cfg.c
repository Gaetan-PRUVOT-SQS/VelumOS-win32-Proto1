#include "velum/util.h"
#include "vblk.h"

static int	vblk_sector_size(t_vblk *v, uint32_t *ss)
{
	uint32_t	bs;
	int			rc;

	*ss = BLK_SS_MIN;
	if (!(v->vio.features & VBLK_F_BLK_SIZE))
		return (0);
	rc = vio_cfg_read(&v->vio, VBLK_CFG_BLK_SIZE, &bs, 4);
	if (rc < 0)
		return (rc);
	if (bs >= BLK_SS_MIN && bs <= BLK_SS_MAX && !(bs & (bs - 1)))
		*ss = bs;
	return (0);
}

static int	vblk_chunk(t_vblk *v, uint32_t ss)
{
	uint32_t	smax;
	int			rc;

	v->chunk = VBLK_BOUNCE_PAGES * PAGE_SIZE / ss;
	if (!(v->vio.features & VBLK_F_SIZE_MAX))
		return (0);
	rc = vio_cfg_read(&v->vio, VBLK_CFG_SIZE_MAX, &smax, 4);
	if (rc < 0)
		return (rc);
	if (smax / ss == 0)
		return (E_NOTSUP);
	if (smax / ss < v->chunk)
		v->chunk = smax / ss;
	return (0);
}

int	vblk_geometry(t_vblk *v)
{
	uint64_t	cap;
	uint32_t	ss;
	int			rc;

	ss = BLK_SS_MIN;
	rc = vio_cfg_read(&v->vio, VBLK_CFG_CAPACITY, &cap, 8);
	if (rc == 0)
		rc = vblk_sector_size(v, &ss);
	if (rc == 0)
		rc = vblk_chunk(v, ss);
	if (rc < 0)
		return (rc);
	v->dev.sector_size = ss;
	v->ss_factor = ss / BLK_SS_MIN;
	v->dev.nsectors = cap / v->ss_factor;
	if (v->dev.nsectors == 0)
		return (E_NODEV);
	if (v->vio.features & VBLK_F_RO)
		v->dev.flags |= BLK_RO;
	return (0);
}

void	vblk_intx_off(const t_pcidev *d)
{
	if (pci_intx_disable(d) < 0)
		klog_warn("virtio-blk: INTx non coupée (bit 10 non retenu)");
}
