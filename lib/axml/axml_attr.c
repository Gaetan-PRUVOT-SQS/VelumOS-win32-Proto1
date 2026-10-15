#include "axml_int.h"

uint32_t	axml_attr_count(const t_axml *x)
{
	if (!x || !x->file.p || x->ev != AXML_START)
		return (0);
	return (x->attr_count);
}

int	axml_attr(const t_axml *x, uint32_t i, t_axmlattr *out)
{
	const uint8_t	*a;

	if (!x || !x->file.p || !out || x->ev != AXML_START)
		return (E_INVAL);
	if (i >= x->attr_count)
		return (E_RANGE);
	a = x->file.p + x->attr_off + (size_t)i * x->attr_size;
	out->ns = res_rd32(a);
	out->name = res_rd32(a + 4);
	out->raw = res_rd32(a + 8);
	out->type = a[15];
	out->data = res_rd32(a + 16);
	out->res_id = 0;
	a = x->file.p + x->map_off;
	if (out->name < x->map_count)
		out->res_id = res_rd32(a + 4 * (size_t)out->name);
	if (out->name >= x->pool.count
		|| (out->raw != AXML_NO_STRING && out->raw >= x->pool.count)
		|| (out->type == RES_T_STRING && out->data >= x->pool.count))
		return (E_INVAL);
	return (E_OK);
}

int	axml_attr_find(const t_axml *x, uint32_t res_id, t_axmlattr *out)
{
	uint32_t	i;
	int			r;

	if (res_id == 0)
		return (E_INVAL);
	i = 0;
	r = axml_attr(x, 0, out);
	while (r == E_OK && out->res_id != res_id)
		r = axml_attr(x, ++i, out);
	if (r == E_RANGE)
		return (E_NOENT);
	return (r);
}

int	axml_name(const t_axml *x, t_text out)
{
	if (!x || !x->file.p || (x->ev != AXML_START && x->ev != AXML_END))
		return (E_INVAL);
	return (respool_get(&x->pool, res_rd32(x->file.p + x->ev_off + 4), out));
}

int	axml_string(const t_axml *x, uint32_t index, t_text out)
{
	if (!x || !x->file.p)
		return (E_INVAL);
	return (respool_get(&x->pool, index, out));
}
