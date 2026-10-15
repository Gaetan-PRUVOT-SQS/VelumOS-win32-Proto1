#include "axml_int.h"

static void	reset(t_axml *x)
{
	x->file.p = NULL;
	x->file.len = 0;
	x->map_off = 0;
	x->map_count = 0;
	x->pos = 0;
	x->depth = 0;
	x->ev = AXML_DONE;
	x->ev_off = 0;
	x->attr_off = 0;
	x->attr_size = 0;
	x->attr_count = 0;
}

int	axml_open(t_axml *x, t_span file)
{
	t_reschunk	c;
	t_span		sub;
	uint32_t	pos;

	if (!x)
		return (E_INVAL);
	reset(x);
	if (res_chunk(file, 0, &c) < 0 || c.type != RES_XML)
		return (E_INVAL);
	file.len = c.size;
	pos = c.hsize;
	if (res_next(file, &pos, &c) != 1 || c.type != RES_STRING_POOL)
		return (E_INVAL);
	sub.p = file.p + c.off;
	sub.len = c.size;
	if (respool_open(&x->pool, sub) < 0)
		return (E_INVAL);
	x->file = file;
	x->pos = pos;
	return (E_OK);
}

int	axml_set_map(t_axml *x, const t_reschunk *c)
{
	if (x->map_off != 0)
		return (E_INVAL);
	x->map_off = c->off + c->hsize;
	x->map_count = (c->size - c->hsize) / 4;
	return (AXML_SKIP);
}

static int	node(t_axml *x, const t_reschunk *c)
{
	if (c->type == RES_XML_RESMAP)
		return (axml_set_map(x, c));
	if (c->type == RES_XML_START_NS || c->type == RES_XML_END_NS)
		return (axml_ev_ns(x, c));
	if (c->type == RES_XML_START)
		return (axml_ev_start(x, c));
	if (c->type == RES_XML_END)
		return (axml_ev_end(x, c));
	if (c->type == RES_XML_CDATA)
		return (axml_ev_text(x, c));
	if (c->type == RES_STRING_POOL)
		return (E_INVAL);
	return (AXML_SKIP);
}

int	axml_next(t_axml *x)
{
	t_reschunk	c;
	int			r;

	if (!x || !x->file.p)
		return (E_INVAL);
	r = res_next(x->file, &x->pos, &c);
	while (r == 1)
	{
		r = node(x, &c);
		if (r < 0)
			x->pos = c.off;
		if (r != AXML_SKIP)
			return (r);
		r = res_next(x->file, &x->pos, &c);
	}
	x->ev = AXML_DONE;
	x->attr_count = 0;
	if (r < 0 || x->depth != 0)
		return (E_INVAL);
	return (AXML_DONE);
}
