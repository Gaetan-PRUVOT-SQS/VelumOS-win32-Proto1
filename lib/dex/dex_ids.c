#include "dex_int.h"

int	dex_string(const t_dex *d, uint32_t idx, t_dexstr *out)
{
	t_dexcur	c;
	t_span		s;
	uint32_t	units;

	if (!d || !out)
		return (E_INVAL);
	if (idx >= d->n[DEX_T_STRING])
		return (E_RANGE);
	c = dex_cur(d, dex_u32(dex_item(d, DEX_T_STRING, idx)));
	out->utf16 = dex_uleb(&c);
	if (c.err)
		return (E_INVAL);
	s.p = d->p + c.pos;
	s.len = d->len - c.pos;
	if (dex_mutf8_check(s, &out->size, &units) != E_OK
		|| units != out->utf16)
		return (E_INVAL);
	out->p = (const char *)s.p;
	return (E_OK);
}

int	dex_type(const t_dex *d, uint32_t type_idx, t_dexstr *out)
{
	if (!d || !out)
		return (E_INVAL);
	if (type_idx >= d->n[DEX_T_TYPE])
		return (E_RANGE);
	return (dex_string(d, dex_u32(dex_item(d, DEX_T_TYPE, type_idx)), out));
}

int	dex_proto(const t_dex *d, uint32_t proto_idx, t_dexproto *out)
{
	const uint8_t	*p;

	if (!d || !out)
		return (E_INVAL);
	if (proto_idx >= d->n[DEX_T_PROTO])
		return (E_RANGE);
	p = dex_item(d, DEX_T_PROTO, proto_idx);
	out->shorty_idx = dex_u32(p);
	out->return_idx = dex_u32(p + 4);
	return (dex_type_list(d, dex_u32(p + 8), &out->params));
}

int	dex_field(const t_dex *d, uint32_t field_idx, t_dexfield *out)
{
	const uint8_t	*p;

	if (!d || !out)
		return (E_INVAL);
	if (field_idx >= d->n[DEX_T_FIELD])
		return (E_RANGE);
	p = dex_item(d, DEX_T_FIELD, field_idx);
	out->class_idx = dex_u16(p);
	out->type_idx = dex_u16(p + 2);
	out->name_idx = dex_u32(p + 4);
	return (E_OK);
}

int	dex_method(const t_dex *d, uint32_t method_idx, t_dexmethod *out)
{
	const uint8_t	*p;

	if (!d || !out)
		return (E_INVAL);
	if (method_idx >= d->n[DEX_T_METHOD])
		return (E_RANGE);
	p = dex_item(d, DEX_T_METHOD, method_idx);
	out->class_idx = dex_u16(p);
	out->proto_idx = dex_u16(p + 2);
	out->name_idx = dex_u32(p + 4);
	return (E_OK);
}
