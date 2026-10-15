#include "velum/util.h"
#include "vblk.h"

static int	vblk_read(t_blkdev *d, uint64_t lba, uint32_t n, void *buf)
{
	t_vblk		*v;
	t_vblkreq	r;
	uint8_t		*dst;
	int			rc;

	v = d->priv;
	dst = buf;
	rc = 0;
	mutex_lock(&v->lock);
	while (rc == 0 && n)
	{
		r.type = VBLK_T_IN;
		r.sector = lba * v->ss_factor;
		r.bytes = (uint32_t)min_u64(n, v->chunk) * d->sector_size;
		rc = vblk_submit(v, &r);
		if (rc == 0)
			memcpy(dst, v->dma + VBLK_BOUNCE_OFF, r.bytes);
		dst += r.bytes;
		lba += r.bytes / d->sector_size;
		n -= r.bytes / d->sector_size;
	}
	mutex_unlock(&v->lock);
	return (rc);
}

static int	vblk_write(t_blkdev *d, uint64_t lba, uint32_t n, const void *buf)
{
	t_vblk			*v;
	t_vblkreq		r;
	const uint8_t	*src;
	int				rc;

	v = d->priv;
	if (d->flags & BLK_RO)
		return (E_PERM);
	src = buf;
	rc = 0;
	mutex_lock(&v->lock);
	while (rc == 0 && n)
	{
		r.type = VBLK_T_OUT;
		r.sector = lba * v->ss_factor;
		r.bytes = (uint32_t)min_u64(n, v->chunk) * d->sector_size;
		memcpy(v->dma + VBLK_BOUNCE_OFF, src, r.bytes);
		rc = vblk_submit(v, &r);
		src += r.bytes;
		lba += r.bytes / d->sector_size;
		n -= r.bytes / d->sector_size;
	}
	mutex_unlock(&v->lock);
	return (rc);
}

static int	vblk_flush(t_blkdev *d)
{
	t_vblk		*v;
	t_vblkreq	r;
	int			rc;

	v = d->priv;
	if (v->dead)
		return (E_IO);
	if (!(v->vio.features & VBLK_F_FLUSH))
		return (0);
	r.type = VBLK_T_FLUSH;
	r.bytes = 0;
	r.sector = 0;
	mutex_lock(&v->lock);
	rc = vblk_submit(v, &r);
	mutex_unlock(&v->lock);
	return (rc);
}

const t_blkops	*vblk_ops(void)
{
	static const t_blkops	ops = {vblk_read, vblk_write, vblk_flush};

	return (&ops);
}
