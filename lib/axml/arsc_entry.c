#include "axml_int.h"

int	arsc_cfg_rank(const uint8_t *cfg, uint32_t n, const char *lang)
{
	uint32_t	i;

	if (lang && lang[0] && cfg[8] == (uint8_t)lang[0]
		&& cfg[9] == (uint8_t)lang[1])
		return (2);
	i = 4;
	while (i < n)
	{
		if (cfg[i] != 0)
			return (0);
		i++;
	}
	return (1);
}

int	arsc_entry_value(const uint8_t *t, const t_reschunk *c, uint64_t pos,
		t_resvalue *v)
{
	uint32_t	esize;

	if (pos + 8 > c->size)
		return (E_INVAL);
	esize = res_rd16(t + pos);
	if (res_rd16(t + pos + 2) & (RES_ENTRY_COMPLEX | RES_ENTRY_COMPACT))
		return (E_NOTSUP);
	if (esize < 8 || pos + esize + 8 > c->size)
		return (E_INVAL);
	v->type = t[pos + esize + 3];
	v->data = res_rd32(t + pos + esize + 4);
	return (E_OK);
}

int	arsc_string(const t_arsc *a, const t_resquery *q, t_text out)
{
	t_resvalue	v;
	uint32_t	id;
	int			depth;
	int			r;

	if (!a || !q)
		return (E_INVAL);
	id = q->res_id;
	depth = 0;
	while (depth++ <= ARSC_MAX_REF)
	{
		r = arsc_lookup(a, id, q->lang, &v);
		if (r < 0)
			return (r);
		if (v.type == RES_T_STRING)
			return (respool_get(&a->strings, v.data, out));
		if (v.type != RES_T_REFERENCE)
			return (E_INVAL);
		id = v.data;
	}
	return (E_RANGE);
}
