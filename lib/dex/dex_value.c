#include "dex_int.h"

static const uint8_t	g_max[32] = {1, 0, 2, 2, 4, 0, 8, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 8, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0xfe, 0xfe, 0xff, 0xff};

static int	head(t_dexcur *c, t_dexvalue *v, uint32_t *n)
{
	uint32_t	b;
	uint32_t	max;

	b = dex_byte(c);
	v->kind = b & 0x1f;
	v->i = b >> 5;
	max = g_max[v->kind];
	*n = (b >> 5) + 1;
	if (c->err || max == 0)
		return (E_INVAL);
	if (max == 0xfe)
		return (E_NOTSUP);
	if (max == 0xff && (b >> 5) > (uint32_t)(v->kind == DEX_V_BOOLEAN))
		return (E_INVAL);
	if (max == 0xff)
		*n = 0;
	if (*n > max)
		return (E_INVAL);
	return (E_OK);
}

static uint64_t	payload(t_dexcur *c, uint32_t n)
{
	uint64_t	v;
	uint32_t	i;

	v = 0;
	i = 0;
	while (i < n)
	{
		v |= (uint64_t)dex_byte(c) << (8 * i);
		i++;
	}
	return (v);
}

static int	decode(const t_dex *d, t_dexvalue *v, uint64_t raw, uint32_t n)
{
	if (v->kind == DEX_V_NULL || v->kind == DEX_V_BOOLEAN)
		return (E_OK);
	if (v->kind > DEX_V_STRING
		|| (v->kind > DEX_V_DOUBLE && v->kind < DEX_V_STRING))
		return (1);
	v->i = (int64_t)raw;
	if (v->kind == DEX_V_FLOAT)
		v->i = (int64_t)(raw << (8 * (4 - n)));
	if (v->kind == DEX_V_DOUBLE)
		v->i = (int64_t)(raw << (8 * (8 - n)));
	if (v->kind <= DEX_V_LONG && v->kind != DEX_V_CHAR && n < 8
		&& ((raw >> (8 * n - 1)) & 1))
		v->i = (int64_t)(raw | (~(uint64_t)0 << (8 * n)));
	if (v->kind == DEX_V_STRING && raw >= d->n[DEX_T_STRING])
		return (E_INVAL);
	return (E_OK);
}

static int	element(const t_dex *d, t_dexcur *c, t_dexvalue *v)
{
	uint32_t	n;
	uint64_t	raw;
	int			r;

	r = head(c, v, &n);
	if (r != E_OK)
		return (r);
	raw = payload(c, n);
	if (c->err)
		return (E_INVAL);
	return (decode(d, v, raw, n));
}

int	dex_static_value(const t_dex *d, const t_dexclass *c, uint32_t ordinal,
		t_dexvalue *out)
{
	t_dexcur	cur;
	uint32_t	i;
	int			r;

	if (!d || !c || !out)
		return (E_INVAL);
	if (c->static_values_off == 0)
		return (E_NOENT);
	cur = dex_cur(d, c->static_values_off);
	i = dex_uleb(&cur);
	if (cur.err)
		return (E_INVAL);
	if (ordinal >= i)
		return (E_NOENT);
	i = 0;
	r = element(d, &cur, out);
	while (r >= 0 && i < ordinal)
	{
		r = element(d, &cur, out);
		i++;
	}
	if (r > 0)
		return (E_NOTSUP);
	return (r);
}
