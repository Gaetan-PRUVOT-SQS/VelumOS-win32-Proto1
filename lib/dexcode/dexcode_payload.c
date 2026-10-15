#include "dexcode_int.h"

static int	array_shape(const uint8_t *p, uint64_t left, t_dpayload *out)
{
	if (left < 4)
		return (E_INVAL);
	out->width = dexcode_unit(p, 1);
	out->count = dexcode_unit(p, 2) | (dexcode_unit(p, 3) << 16);
	if (out->width != 1 && out->width != 2 && out->width != 4
		&& out->width != 8)
		return (E_INVAL);
	out->units = 4 + ((uint64_t)out->count * out->width + 1) / 2;
	return (0);
}

int	dexcode_payload(t_span insns, uint32_t pc, t_dpayload *out)
{
	const uint8_t	*p;
	uint64_t		left;
	int				r;

	if (!insns.p || !out || (pc & 1) || (uint64_t)pc + 2 > insns.len / 2)
		return (E_INVAL);
	p = insns.p + 2 * (size_t)pc;
	left = insns.len / 2 - pc;
	out->ident = dexcode_unit(p, 0);
	out->count = dexcode_unit(p, 1);
	out->width = 0;
	out->units = 0;
	r = 0;
	if (out->ident == DP_PACKED)
		out->units = 4 + 2 * (uint64_t)out->count;
	else if (out->ident == DP_SPARSE)
		out->units = 2 + 4 * (uint64_t)out->count;
	else if (out->ident == DP_ARRAY)
		r = array_shape(p, left, out);
	else
		r = E_INVAL;
	if (r < 0 || out->units > left)
		return (E_INVAL);
	return (0);
}

int	dexcode_switch(t_span insns, uint32_t payload_pc, t_dswitch *out)
{
	t_dpayload		pl;
	const uint8_t	*p;

	if (!out || dexcode_payload(insns, payload_pc, &pl) < 0
		|| pl.ident == DP_ARRAY)
		return (E_INVAL);
	p = insns.p + 2 * (size_t)payload_pc;
	out->sparse = (pl.ident == DP_SPARSE);
	out->count = pl.count;
	out->keys = NULL;
	out->targets = p + 8;
	out->first_key = 0;
	if (!out->sparse || pl.count > 0)
		out->first_key = dexcode_rd32(p + 4);
	if (out->sparse)
	{
		out->keys = p + 4;
		out->targets = p + 4 + 4 * (size_t)pl.count;
	}
	return (0);
}

int	dexcode_array_data(t_span insns, uint32_t payload_pc, t_darray *out)
{
	t_dpayload	pl;

	if (!out || dexcode_payload(insns, payload_pc, &pl) < 0
		|| pl.ident != DP_ARRAY)
		return (E_INVAL);
	out->width = pl.width;
	out->count = pl.count;
	out->data = insns.p + 2 * (size_t)payload_pc + 8;
	return (0);
}
