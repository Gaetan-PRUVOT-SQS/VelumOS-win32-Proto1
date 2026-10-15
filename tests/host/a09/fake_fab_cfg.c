#include "fake_fab.h"

void	fab_bar(t_fabdev *d, uint8_t idx, uint8_t kind, uint64_t size)
{
	d->bar[idx].kind = kind;
	d->bar[idx].size = size;
	if (kind == FAB_MEM64_LO && idx + 1 < FAB_BARS)
	{
		d->bar[idx + 1].kind = FAB_MEM64_HI;
		d->bar[idx + 1].size = size;
	}
}

void	fab_bar_base(t_fabdev *d, uint8_t idx, uint64_t base)
{
	fab_bar_store(d, idx, (uint32_t)base);
	if (d->bar[idx].kind == FAB_MEM64_LO)
		fab_bar_store(d, (uint8_t)(idx + 1), (uint32_t)(base >> 32));
}

void	fab_cap(t_fabdev *d, uint8_t off, uint8_t id, uint8_t next)
{
	fab_put(d, 0x06, 2, fab_get(d, 0x06, 2) | 0x10);
	fab_put(d, off, 1, id);
	fab_put(d, off + 1u, 1, next);
}

void	fab_bridge(t_fabdev *d, uint8_t sec, uint8_t sub)
{
	fab_put(d, 0x18, 1, d->bus);
	fab_put(d, 0x19, 1, sec);
	fab_put(d, 0x1a, 1, sub);
}

void	fab_irq(t_fabdev *d, uint8_t line, uint8_t pin)
{
	fab_put(d, 0x3c, 1, line);
	fab_put(d, 0x3d, 1, pin);
}
