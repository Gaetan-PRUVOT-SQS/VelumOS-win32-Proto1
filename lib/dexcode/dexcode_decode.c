#include "dexcode_int.h"

static void	fill_args(const uint8_t *p, t_dinsn *out)
{
	uint32_t	w2;

	w2 = dexcode_unit(p, 2);
	out->args[0] = w2 & 15;
	out->args[1] = (w2 >> 4) & 15;
	out->args[2] = (w2 >> 8) & 15;
	out->args[3] = w2 >> 12;
	out->args[4] = (uint16_t)dexcode_field(p, DS_A4);
}

static void	fill(const uint8_t *p, const t_dfmtinfo *f, t_dinsn *out)
{
	out->len = f->len;
	out->a = dexcode_field(p, f->a);
	out->b = dexcode_field(p, f->b);
	out->c = dexcode_field(p, f->c);
	out->lit = dexcode_literal(p, f->lit);
	out->idx = dexcode_field(p, f->idx);
	out->argc = (uint8_t)dexcode_field(p, f->argc);
	out->idx2 = 0;
	if (f->len == 4)
		out->idx2 = dexcode_unit(p, 3);
	if (f->regs & DR_LIST)
		fill_args(p, out);
	if (out->fmt == DF_21H && out->op == OP_CONST_WIDE_HIGH16)
		out->lit = (int64_t)((uint64_t)out->lit << 48);
	else if (out->fmt == DF_21H)
		out->lit = (int64_t)((uint64_t)out->lit << 16);
}

uint8_t	dexcode_format_len(uint8_t fmt)
{
	return (dexcode_fmtinfo(fmt)->len);
}

int	dexcode_decode(t_span insns, uint32_t pc, t_dinsn *out)
{
	const t_dfmtinfo	*f;
	const uint8_t		*p;
	int					i;

	if (!insns.p || !out || pc >= insns.len / 2)
		return (E_INVAL);
	p = insns.p + 2 * (size_t)pc;
	out->op = p[0];
	out->fmt = g_dops[p[0]].fmt;
	f = dexcode_fmtinfo(out->fmt);
	if (f->len == 0 || (uint64_t)pc + f->len > insns.len / 2)
		return (E_INVAL);
	i = 0;
	while (i < 5)
		out->args[i++] = 0;
	fill(p, f, out);
	if ((f->regs & DR_LIST) && out->argc > 5)
		return (E_INVAL);
	return (0);
}
