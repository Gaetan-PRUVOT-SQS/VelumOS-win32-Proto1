#include "axml_int.h"

static int	body(const t_reschunk *c, uint32_t need)
{
	if (c->hsize < 16 || c->size - c->hsize < need)
		return (E_INVAL);
	return (E_OK);
}

int	axml_ev_ns(t_axml *x, const t_reschunk *c)
{
	(void)x;
	if (body(c, 8) < 0)
		return (E_INVAL);
	return (AXML_SKIP);
}

int	axml_ev_start(t_axml *x, const t_reschunk *c)
{
	const uint8_t	*e;
	uint64_t		span;

	if (body(c, 20) < 0)
		return (E_INVAL);
	e = x->file.p + c->off + c->hsize;
	if (res_rd32(e + 4) >= x->pool.count)
		return (E_INVAL);
	span = (uint64_t)res_rd16(e + 12) * res_rd16(e + 10) + res_rd16(e + 8);
	if (res_rd16(e + 8) < 20 || res_rd16(e + 10) < 20
		|| span > c->size - c->hsize)
		return (E_INVAL);
	if (x->depth >= AXML_MAX_DEPTH)
		return (E_RANGE);
	x->depth++;
	x->ev = AXML_START;
	x->ev_off = c->off + c->hsize;
	x->attr_off = x->ev_off + res_rd16(e + 8);
	x->attr_size = res_rd16(e + 10);
	x->attr_count = res_rd16(e + 12);
	return (AXML_START);
}

int	axml_ev_end(t_axml *x, const t_reschunk *c)
{
	if (body(c, 8) < 0 || x->depth == 0)
		return (E_INVAL);
	if (res_rd32(x->file.p + c->off + c->hsize + 4) >= x->pool.count)
		return (E_INVAL);
	x->depth--;
	x->ev = AXML_END;
	x->ev_off = c->off + c->hsize;
	x->attr_count = 0;
	return (AXML_END);
}

int	axml_ev_text(t_axml *x, const t_reschunk *c)
{
	if (body(c, 12) < 0)
		return (E_INVAL);
	if (res_rd32(x->file.p + c->off + c->hsize) >= x->pool.count)
		return (E_INVAL);
	x->ev = AXML_TEXT;
	x->ev_off = c->off + c->hsize;
	x->attr_count = 0;
	return (AXML_TEXT);
}
