#include "dexcode_int.h"

static int	keys_ok(const t_dscan *s, const t_dpayload *pl)
{
	const uint8_t	*p;
	uint32_t		i;

	p = s->insns.p + 2 * (size_t)s->pc + 4;
	if (pl->ident == DP_PACKED)
		return (pl->count == 0
			|| (int64_t)dexcode_rd32(p) + pl->count - 1 <= INT32_MAX);
	i = 1;
	while (pl->ident == DP_SPARSE && i < pl->count)
	{
		if (dexcode_rd32(p + 4 * (size_t)i)
			<= dexcode_rd32(p + 4 * (size_t)i - 4))
			return (0);
		i++;
	}
	return (1);
}

static int	scan_payload(t_dscan *s)
{
	t_dpayload	pl;

	if (s->live == 1 || dexcode_payload(s->insns, s->pc, &pl) < 0)
		return (E_INVAL);
	if (!keys_ok(s, &pl))
		return (E_INVAL);
	s->pc += (uint32_t)pl.units;
	s->live = 0;
	return (0);
}

static int	scan_insn(t_dscan *s)
{
	t_dinsn		in;
	uint16_t	fl;
	int			r;

	r = dexcode_decode(s->insns, s->pc, &in);
	if (r < 0)
		return (r);
	fl = dexcode_flags(in.op);
	if (fl & DO_NOTSUP)
		return (E_NOTSUP);
	r = dexcode_check(&in, s->lim);
	if (r < 0)
		return (r);
	if (in.op == OP_NOP && s->live == 0)
		s->live = 2;
	else
		s->live = (fl & DO_CONT) != 0;
	s->pc += in.len;
	return (0);
}

int	dexcode_scan(t_dscan *s)
{
	int	r;

	s->pc = 0;
	s->live = 1;
	while (s->pc < s->n)
	{
		s->starts[s->pc >> 3] |= (uint8_t)(1u << (s->pc & 7));
		if (dexcode_is_payload(s, s->pc))
			r = scan_payload(s);
		else
			r = scan_insn(s);
		if (r < 0)
			return (r);
	}
	if (s->live != 0)
		return (E_INVAL);
	return (0);
}

int	dexcode_verify(t_span insns, const t_dcodelimits *lim, uint8_t *starts)
{
	t_dscan		s;
	uint64_t	i;
	int			r;

	if (!insns.p || !lim || !starts || insns.len == 0 || (insns.len & 1)
		|| insns.len / 2 > UINT32_MAX || lim->ins > lim->registers)
		return (E_INVAL);
	s.insns = insns;
	s.lim = lim;
	s.starts = starts;
	s.n = (uint32_t)(insns.len / 2);
	i = 0;
	while (i < ((uint64_t)s.n + 7) / 8)
		starts[i++] = 0;
	r = dexcode_scan(&s);
	if (r == 0)
		r = dexcode_targets(&s);
	if (r == 0)
		dexcode_strip(&s);
	return (r);
}
