#include "dexcode_int.h"

static int	index_ok(uint8_t kind, uint32_t idx, const t_dcodelimits *lim)
{
	if (kind == DK_STRING)
		return (idx < lim->strings);
	if (kind == DK_TYPE)
		return (idx < lim->types);
	if (kind == DK_FIELD)
		return (idx < lim->fields);
	if (kind == DK_METHOD)
		return (idx < lim->methods);
	if (kind == DK_PROTO)
		return (idx < lim->protos);
	return (kind == DK_NONE);
}

static int	reg_ok(uint32_t r, int wide, uint32_t nreg)
{
	return ((uint64_t)r + (wide != 0) < nreg);
}

static int	regs_ok(const t_dinsn *in, uint16_t fl, uint32_t nreg)
{
	uint8_t	mask;
	uint8_t	i;

	mask = dexcode_fmtinfo(in->fmt)->regs;
	if ((mask & DR_A) && !reg_ok(in->a, fl & (DO_WDST | DO_WA), nreg))
		return (0);
	if ((mask & DR_B) && !reg_ok(in->b, fl & DO_WB, nreg))
		return (0);
	if ((mask & DR_C) && !reg_ok(in->c, fl & DO_WC, nreg))
		return (0);
	if (mask & DR_RANGE)
		return (in->argc == 0 || (uint64_t)in->c + in->argc <= nreg);
	i = 0;
	while ((mask & DR_LIST) && i < in->argc)
	{
		if (in->args[i] >= nreg)
			return (0);
		i++;
	}
	return (1);
}

int	dexcode_check(const t_dinsn *in, const t_dcodelimits *lim)
{
	const t_dopinfo	*op;

	op = &g_dops[in->op];
	if (!regs_ok(in, op->flags, lim->registers))
		return (E_RANGE);
	if (!index_ok(op->kind, in->idx, lim))
		return (E_RANGE);
	if ((op->flags & DO_INVOKE) && in->argc > lim->outs)
		return (E_RANGE);
	return (0);
}
