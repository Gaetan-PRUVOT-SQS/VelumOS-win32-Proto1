#include "axml_int.h"

int	respool_open(t_respool *p, t_span chunk)
{
	t_reschunk	c;
	uint64_t	need;

	if (!p || res_chunk(chunk, 0, &c) < 0)
		return (E_INVAL);
	if (c.type != RES_STRING_POOL || c.hsize < RES_POOL_HEADER)
		return (E_INVAL);
	p->p = chunk.p;
	p->size = c.size;
	p->hsize = c.hsize;
	p->count = res_rd32(chunk.p + 8);
	p->utf8 = (res_rd32(chunk.p + 16) & RES_POOL_UTF8) != 0;
	p->strings = res_rd32(chunk.p + 20);
	need = (uint64_t)c.hsize + (uint64_t)p->count * 4;
	if (need > c.size)
		return (E_INVAL);
	if (p->count && (p->strings < need || p->strings >= c.size))
		return (E_INVAL);
	return (E_OK);
}

int	respool_get(const t_respool *p, uint32_t index, t_text out)
{
	uint64_t	off;

	if (!p || !p->p || !out.p || out.cap == 0)
		return (E_INVAL);
	if (index >= p->count)
		return (E_RANGE);
	if (out.cap > RES_SIZE_MAX)
		out.cap = RES_SIZE_MAX;
	off = res_rd32(p->p + p->hsize + 4 * (size_t)index);
	off += p->strings;
	if (off >= p->size)
		return (E_INVAL);
	if (p->utf8)
		return (respool_u8(p, (uint32_t)off, out));
	return (respool_u16(p, (uint32_t)off, out));
}
