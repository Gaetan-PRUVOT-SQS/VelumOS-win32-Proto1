#include "dex_int.h"

uint32_t	dex_u16(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8));
}

uint32_t	dex_u32(const uint8_t *p)
{
	return (dex_u16(p) | (dex_u16(p + 2) << 16));
}

int	dex_fits(const t_dex *d, uint64_t off, uint64_t n, uint64_t size)
{
	if (off > d->len)
		return (0);
	return (n <= (d->len - off) / size);
}

t_dexcur	dex_cur(const t_dex *d, uint64_t off)
{
	t_dexcur	c;

	c.p = d->p;
	c.len = d->len;
	c.pos = 0;
	c.err = (off >= d->len);
	if (!c.err)
		c.pos = (size_t)off;
	return (c);
}

uint32_t	dex_byte(t_dexcur *c)
{
	if (c->err || c->pos >= c->len)
	{
		c->err = 1;
		return (0);
	}
	return (c->p[c->pos++]);
}
