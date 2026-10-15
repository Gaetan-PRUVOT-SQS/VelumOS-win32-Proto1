#include "dex_int.h"

static int	idx_ok(uint32_t idx, uint32_t n)
{
	return (idx == DEX_NO_INDEX || idx < n);
}

static int	same(const char *a, const char *b)
{
	size_t	i;

	i = 0;
	while (a[i] != 0 && a[i] == b[i])
		i++;
	return (a[i] == b[i]);
}

int	dex_class(const t_dex *d, uint32_t def_idx, t_dexclass *out)
{
	const uint8_t	*p;

	if (!d || !out)
		return (E_INVAL);
	if (def_idx >= d->n[DEX_T_CLASS])
		return (E_RANGE);
	p = dex_item(d, DEX_T_CLASS, def_idx);
	out->class_idx = dex_u32(p);
	out->access = dex_u32(p + 4);
	out->superclass_idx = dex_u32(p + 8);
	out->source_file_idx = dex_u32(p + 16);
	out->class_data_off = dex_u32(p + 24);
	out->static_values_off = dex_u32(p + 28);
	if (out->class_idx >= d->n[DEX_T_TYPE]
		|| !idx_ok(out->superclass_idx, d->n[DEX_T_TYPE])
		|| !idx_ok(out->source_file_idx, d->n[DEX_T_STRING]))
		return (E_INVAL);
	if (dex_u32(p + 20) >= d->len || out->class_data_off >= d->len
		|| out->static_values_off >= d->len)
		return (E_INVAL);
	return (dex_type_list(d, dex_u32(p + 12), &out->interfaces));
}

int	dex_find_class(const t_dex *d, const char *descriptor)
{
	uint32_t	i;
	t_dexstr	s;

	if (!d || !descriptor)
		return (E_INVAL);
	i = 0;
	while (i < d->n[DEX_T_CLASS])
	{
		if (dex_type(d, dex_u32(dex_item(d, DEX_T_CLASS, i)), &s) == E_OK
			&& same(s.p, descriptor))
			return ((int)i);
		i++;
	}
	return (E_NOENT);
}

int	dex_verify_classes(const t_dex *d)
{
	uint32_t	i;
	t_dexclass	c;

	i = 0;
	while (i < d->n[DEX_T_CLASS])
	{
		if (dex_class(d, i, &c) != E_OK)
			return (E_INVAL);
		i++;
	}
	return (E_OK);
}
