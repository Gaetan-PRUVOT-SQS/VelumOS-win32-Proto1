#include "dexcode_int.h"

static int	target_ok(const t_dscan *s, int64_t t)
{
	if (t < 0 || t >= s->n || !dexcode_bit(s->starts, (uint32_t)t))
		return (0);
	if (dexcode_is_payload(s, (uint32_t)t))
		return (0);
	if (dexcode_unit(s->insns.p, (uint64_t)t) == 0 && t + 1 < s->n
		&& dexcode_is_payload(s, (uint32_t)t + 1))
		return (0);
	return (1);
}

static int	data_ok(const t_dscan *s, const t_dinsn *in)
{
	t_dpayload	pl;
	t_dswitch	sw;
	int64_t		t;
	uint32_t	i;

	t = (int64_t)s->pc + in->lit;
	if (t < 0 || t >= s->n || !dexcode_bit(s->starts, (uint32_t)t)
		|| dexcode_payload(s->insns, (uint32_t)t, &pl) < 0)
		return (E_INVAL);
	if ((in->op == OP_FILL_ARRAY_DATA) != (pl.ident == DP_ARRAY)
		|| (in->op == OP_PACKED_SWITCH) != (pl.ident == DP_PACKED))
		return (E_INVAL);
	if (pl.ident == DP_ARRAY || dexcode_switch(s->insns, (uint32_t)t, &sw) < 0)
		return (0);
	i = 0;
	while (i < sw.count)
	{
		t = (int64_t)s->pc + dexcode_rd32(sw.targets + 4 * (size_t)i);
		if (!target_ok(s, t))
			return (E_INVAL);
		i++;
	}
	return (0);
}

static int	insn_targets(const t_dscan *s)
{
	t_dinsn		in;
	uint16_t	fl;

	if (dexcode_decode(s->insns, s->pc, &in) < 0)
		return (E_INVAL);
	fl = dexcode_flags(in.op);
	if ((fl & DO_BRANCH) && in.lit == 0 && in.op != OP_GOTO_32)
		return (E_INVAL);
	if ((fl & DO_BRANCH) && !target_ok(s, (int64_t)s->pc + in.lit))
		return (E_INVAL);
	if (fl & (DO_SWITCH | DO_FILL))
		return (data_ok(s, &in));
	return (0);
}

int	dexcode_targets(t_dscan *s)
{
	uint32_t	len;

	s->pc = 0;
	while (s->pc < s->n)
	{
		len = dexcode_item_len(s, s->pc);
		if (len == 0)
			return (E_INVAL);
		if (!dexcode_is_payload(s, s->pc) && insn_targets(s) < 0)
			return (E_INVAL);
		s->pc += len;
	}
	return (0);
}
