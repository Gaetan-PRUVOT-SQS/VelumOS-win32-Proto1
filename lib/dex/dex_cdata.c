#include "dex_int.h"

int	dex_cdata_open(const t_dex *d, const t_dexclass *c, t_dexcdata *it)
{
	t_dexcur	cur;
	uint32_t	i;

	if (!d || !c || !it)
		return (E_INVAL);
	it->d = d;
	it->kind = 4;
	it->last = 0;
	it->pos = 0;
	if (c->class_data_off == 0)
		return (E_OK);
	cur = dex_cur(d, c->class_data_off);
	i = 0;
	while (i < 4)
	{
		it->left[i] = dex_uleb(&cur);
		if (cur.err || it->left[i] > d->len)
			return (E_INVAL);
		i++;
	}
	it->pos = (uint32_t)cur.pos;
	it->kind = 0;
	return (E_OK);
}

static int	member(t_dexcdata *it, t_dexcur *c, t_dexmember *out)
{
	uint64_t	idx;
	uint32_t	limit;

	idx = (uint64_t)it->last + dex_uleb(c);
	out->access = dex_uleb(c);
	out->code_off = 0;
	limit = it->d->n[DEX_T_FIELD];
	if (it->kind >= DEX_M_DIRECT)
	{
		limit = it->d->n[DEX_T_METHOD];
		out->code_off = dex_uleb(c);
	}
	if (c->err || idx >= limit || out->code_off >= it->d->len)
		return (E_INVAL);
	out->kind = it->kind;
	out->idx = (uint32_t)idx;
	return (E_OK);
}

int	dex_cdata_next(t_dexcdata *it, t_dexmember *out)
{
	t_dexcur	c;

	if (!it || !out)
		return (E_INVAL);
	while (it->kind < 4 && it->left[it->kind] == 0)
	{
		it->kind++;
		it->last = 0;
	}
	if (it->kind >= 4)
		return (E_NOENT);
	c = dex_cur(it->d, it->pos);
	if (member(it, &c, out) != E_OK)
	{
		it->kind = 4;
		return (E_INVAL);
	}
	it->left[it->kind]--;
	it->last = out->idx;
	it->pos = (uint32_t)c.pos;
	return (E_OK);
}
