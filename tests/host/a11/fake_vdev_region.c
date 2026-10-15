#include <stdlib.h>
#include "fake.h"
#include "virtio.h"

static uint32_t	fk_devcfg_rd(t_fkdev *f, uint32_t off, uint32_t width)
{
	uint32_t	v;
	uint32_t	i;

	if (off + width > sizeof(f->devcfg))
		return (0);
	v = 0;
	i = 0;
	while (i < width)
	{
		v |= (uint32_t)f->devcfg[off + i] << (8 * i);
		i++;
	}
	return (v);
}

uint32_t	fk_region_rd(t_fkdev *f, uint32_t off, uint32_t width)
{
	uint32_t	v;

	if (off < FK_ISR)
	{
		v = fk_common_rd(f, off);
		if (width < 4)
			v &= (1u << (8 * width)) - 1;
		return (v);
	}
	if (off >= FK_DEVCFG && off < FK_DEVCFG + FK_REGION)
		return (fk_devcfg_rd(f, off - FK_DEVCFG, width));
	if (off >= FK_NOTIFY)
		abort();
	return (0);
}

void	fk_region_wr(t_fkdev *f, uint32_t off, uint32_t v)
{
	if (off < FK_ISR)
	{
		fk_common_wr(f, off, v);
		return ;
	}
	if (off != FK_NOTIFY + (uint32_t)f->notify_off * FK_MULT || v != 0)
		abort();
	if (f->modes & FK_DELAY)
		f->pending = true;
	else
		fk_process(f);
}
