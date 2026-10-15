#include "fake.h"
#include "virtio.h"

static void	fk_reset(t_fkdev *f)
{
	if (f->modes & FK_RESET_STUCK)
	{
		f->status = VIO_ST_ACK;
		return ;
	}
	f->status = 0;
	f->drv_features = 0;
	f->qenable = 0;
	f->qaddr[0] = 0;
	f->qaddr[1] = 0;
	f->qaddr[2] = 0;
	f->qsize = f->qmax;
	f->last_avail = 0;
	f->pending = false;
}

static void	fk_status_wr(t_fkdev *f, uint8_t v)
{
	bool	ok;

	if (v == 0)
	{
		fk_reset(f);
		return ;
	}
	if ((v & VIO_ST_FEATURES_OK) && !(f->status & VIO_ST_FEATURES_OK))
	{
		ok = !(f->modes & FK_REFUSE_FEAT)
			&& !(f->drv_features & ~f->features)
			&& (f->drv_features & VIO_F_VERSION_1);
		if (!ok)
			v = (uint8_t)(v & ~VIO_ST_FEATURES_OK);
	}
	f->status = v;
}

static void	fk_half(uint64_t *dst, uint32_t high, uint32_t v)
{
	if (high)
		*dst = (*dst & 0xffffffffull) | ((uint64_t)v << 32);
	else
		*dst = (*dst & ~0xffffffffull) | v;
}

void	fk_common_wr(t_fkdev *f, uint32_t off, uint32_t v)
{
	if (off == VIO_CC_DFSELECT)
		f->dfsel = v;
	else if (off == VIO_CC_GFSELECT)
		f->gfsel = v;
	else if (off == VIO_CC_GF && f->gfsel < 2)
		fk_half(&f->drv_features, f->gfsel, v);
	else if (off == VIO_CC_STATUS)
		fk_status_wr(f, (uint8_t)v);
	else if (off == VIO_CC_QSELECT)
		f->qsel = (uint16_t)v;
	else if (off == VIO_CC_QSIZE && f->qsel == 0)
		f->qsize = (uint16_t)v;
	else if (off == VIO_CC_QENABLE && f->qsel == 0)
		f->qenable = (uint16_t)v;
	else if (off >= VIO_CC_QDESC && off < VIO_CC_QDEVICE + 8 && f->qsel == 0)
		fk_half(&f->qaddr[(off - VIO_CC_QDESC) / 8], (off & 4) != 0, v);
}

uint32_t	fk_common_rd(t_fkdev *f, uint32_t off)
{
	if (off == VIO_CC_DF && f->dfsel < 2)
		return ((uint32_t)(f->features >> (32 * f->dfsel)));
	if (off == VIO_CC_NUMQ)
		return (1);
	if (off == VIO_CC_STATUS)
		return (f->status);
	if (off == VIO_CC_GENERATION && (f->modes & FK_GEN_FLAP))
		return (f->gen++);
	if (off == VIO_CC_GENERATION)
		return (f->gen);
	if (off == VIO_CC_QSIZE && f->qsel == 0)
		return (f->qsize);
	if (off == VIO_CC_QENABLE && f->qsel == 0)
		return (f->qenable);
	if (off == VIO_CC_QNOTIFY_OFF && f->qsel == 0)
		return (f->notify_off);
	return (0);
}
