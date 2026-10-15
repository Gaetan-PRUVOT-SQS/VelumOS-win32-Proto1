#include "axml_int.h"

static int	type_pick(const t_arsc *a, const t_reschunk *c, t_arscpick *k)
{
	const uint8_t	*t;
	uint32_t		n;
	uint32_t		e;
	int				rank;

	t = a->file.p + c->off;
	if (c->hsize < 32 || res_rd32(t + 20) < 12
		|| res_rd32(t + 20) > c->hsize - 20)
		return (E_INVAL);
	n = res_rd32(t + 12);
	e = k->res_id & 0xffff;
	if ((uint64_t)c->hsize + (uint64_t)n * 4 > c->size)
		return (E_INVAL);
	rank = arsc_cfg_rank(t + 20, res_rd32(t + 20), k->lang);
	if (t[8] != ((k->res_id >> 16) & 0xff) || e >= n || rank <= k->rank)
		return (E_OK);
	if (t[9] & 3)
		return (E_NOTSUP);
	e = res_rd32(t + c->hsize + 4 * (size_t)e);
	if (e == RES_NO_ENTRY)
		return (E_OK);
	k->rank = rank;
	return (arsc_entry_value(t, c, (uint64_t)res_rd32(t + 16) + e, &k->val));
}

static int	pkg_scan(const t_arsc *a, const t_reschunk *pkg, t_arscpick *k)
{
	t_span		s;
	t_reschunk	c;
	uint32_t	pos;
	int			r;

	s.p = a->file.p;
	s.len = pkg->off + pkg->size;
	pos = pkg->off + pkg->hsize;
	r = res_next(s, &pos, &c);
	while (r == 1)
	{
		r = E_OK;
		if (c.type == RES_TABLE_TYPE)
			r = type_pick(a, &c, k);
		if (r == E_OK)
			r = res_next(s, &pos, &c);
	}
	return (r);
}

static int	top_scan(const t_arsc *a, t_arscpick *k)
{
	t_reschunk	c;
	uint32_t	pos;
	int			r;

	pos = a->body;
	r = res_next(a->file, &pos, &c);
	while (r == 1)
	{
		r = E_OK;
		if (c.type == RES_TABLE_PACKAGE
			&& res_rd32(a->file.p + c.off + 8) == k->res_id >> 24)
			r = pkg_scan(a, &c, k);
		if (r == E_OK)
			r = res_next(a->file, &pos, &c);
	}
	return (r);
}

int	arsc_lookup(const t_arsc *a, uint32_t res_id, const char *lang,
		t_resvalue *out)
{
	t_arscpick	k;
	int			r;

	if (!a || !a->file.p || !out)
		return (E_INVAL);
	k.res_id = res_id;
	k.lang = lang;
	k.rank = 0;
	r = top_scan(a, &k);
	if (r < 0)
		return (r);
	if (k.rank == 0)
		return (E_NOENT);
	*out = k.val;
	return (E_OK);
}
