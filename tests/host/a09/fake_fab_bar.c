#include "fake_fab.h"

static uint32_t	bar_mask(const t_fabdev *d, uint8_t idx)
{
	const t_fabbar	*b;

	b = &d->bar[idx];
	if (b->kind == FAB_MEM32 || b->kind == FAB_MEM64_LO)
		return (~(uint32_t)(b->size - 1) & ~0xfu);
	if (b->kind == FAB_MEM64_HI)
		return ((uint32_t)(~(b->size - 1) >> 32));
	if (b->kind == FAB_IO)
		return (~(uint32_t)(b->size - 1) & ~3u);
	return (0);
}

void	fab_bar_store(t_fabdev *d, uint8_t idx, uint32_t v)
{
	fab_put(d, (uint16_t)(0x10 + 4 * idx), 4, v & bar_mask(d, idx));
}

uint32_t	fab_bar_reg(const t_fabdev *d, uint8_t idx)
{
	uint32_t	v;
	uint8_t		kind;

	kind = d->bar[idx].kind;
	v = fab_get(d, (uint16_t)(0x10 + 4 * idx), 4);
	if (kind == FAB_NONE)
		return (0);
	if (kind == FAB_ONES)
		return (0xffffffffu);
	if (kind == FAB_IO)
		v |= 1;
	if (kind == FAB_MEM64_LO)
		v |= 4;
	if ((kind == FAB_MEM32 || kind == FAB_MEM64_LO) && d->bar[idx].pref)
		v |= 8;
	return (v);
}

int	fab_in_bars(const t_fabdev *d, uint16_t off)
{
	uint16_t	end;

	end = 0x10 + 4 * 6;
	if (d->cfg[0x0e] & 0x7f)
		end = 0x10 + 4 * 2;
	return (off >= 0x10 && off < end);
}
