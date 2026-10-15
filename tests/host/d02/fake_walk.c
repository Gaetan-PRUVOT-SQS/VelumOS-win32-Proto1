#include "fx.h"

static void	walk_attrs(const t_axml *x)
{
	t_axmlattr	a;
	char		buf[48];
	t_text		t;
	uint32_t	i;

	t.p = buf;
	t.cap = sizeof(buf);
	axml_name(x, t);
	axml_text(x, t);
	i = 0;
	while (i < axml_attr_count(x))
	{
		if (axml_attr(x, i, &a) == 0 && a.raw != AXML_NO_STRING)
			axml_string(x, a.raw, t);
		i++;
	}
	axml_attr_find(x, AXML_ATTR_NAME, &a);
}

int	fx_walk_axml(t_span s)
{
	t_axml	x;
	int		n;
	int		r;

	r = axml_open(&x, s);
	if (r < 0)
		return (r);
	n = 0;
	r = axml_next(&x);
	while (r > 0 && n < 1000000)
	{
		walk_attrs(&x);
		n++;
		r = axml_next(&x);
	}
	return (r);
}

static int	walk_pool(t_span s)
{
	t_respool	p;
	char		buf[64];
	t_text		t;
	uint32_t	i;
	int			r;

	r = respool_open(&p, s);
	t.p = buf;
	t.cap = sizeof(buf);
	i = 0;
	while (r == 0 && i < p.count && i < 4096)
		respool_get(&p, i++, t);
	return (r);
}

static int	walk_arsc(t_span s)
{
	t_arsc		a;
	t_resquery	q;
	t_resvalue	v;
	char		buf[64];
	t_text		t;

	if (arsc_open(&a, s) < 0)
		return (E_INVAL);
	t.p = buf;
	t.cap = sizeof(buf);
	q.res_id = 0x7f020000;
	while (q.res_id < 0x7f020012)
	{
		q.lang = NULL;
		arsc_string(&a, &q, t);
		q.lang = "fr";
		arsc_string(&a, &q, t);
		arsc_lookup(&a, q.res_id, "fr", &v);
		q.res_id++;
	}
	return (0);
}

int	fx_walk(int kind, t_span s)
{
	if (kind == FX_POOL)
		return (walk_pool(s));
	if (kind == FX_AXML)
		return (fx_walk_axml(s));
	return (walk_arsc(s));
}
