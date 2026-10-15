#include "axml_int.h"

static int	pkg_pool(const t_arsc *a, const t_reschunk *c, uint32_t field)
{
	t_respool	p;
	t_span		s;
	uint32_t	off;

	off = res_rd32(a->file.p + c->off + field);
	if (off < c->hsize || off >= c->size)
		return (E_INVAL);
	s.p = a->file.p + c->off + off;
	s.len = c->size - off;
	return (respool_open(&p, s));
}

static int	top(t_arsc *a, const t_reschunk *c)
{
	t_span	s;

	if (c->type == RES_STRING_POOL)
	{
		if (a->has_pool)
			return (E_INVAL);
		a->has_pool = 1;
		s.p = a->file.p + c->off;
		s.len = c->size;
		return (respool_open(&a->strings, s));
	}
	if (c->type != RES_TABLE_PACKAGE)
		return (E_OK);
	if (c->hsize < 284 || pkg_pool(a, c, 268) < 0 || pkg_pool(a, c, 276) < 0)
		return (E_INVAL);
	a->packages++;
	return (E_OK);
}

int	arsc_open(t_arsc *a, t_span file)
{
	t_reschunk	c;
	uint32_t	pos;
	int			r;

	if (!a || res_chunk(file, 0, &c) < 0)
		return (E_INVAL);
	a->file.p = file.p;
	a->file.len = c.size;
	a->body = c.hsize;
	a->has_pool = 0;
	a->packages = 0;
	pos = c.hsize;
	r = E_INVAL;
	if (c.type == RES_TABLE && c.hsize >= 12)
		r = res_next(a->file, &pos, &c);
	while (r == 1)
	{
		r = top(a, &c);
		if (r == E_OK)
			r = res_next(a->file, &pos, &c);
	}
	if (r == E_OK && a->has_pool)
		return (E_OK);
	a->file.p = NULL;
	return (E_INVAL);
}

int	axml_text(const t_axml *x, t_text out)
{
	if (!x || !x->file.p || x->ev != AXML_TEXT)
		return (E_INVAL);
	return (respool_get(&x->pool, res_rd32(x->file.p + x->ev_off), out));
}
