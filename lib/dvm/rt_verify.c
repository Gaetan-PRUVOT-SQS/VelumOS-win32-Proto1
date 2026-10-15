#include "rt_int.h"

static int	rt_start(const uint8_t *starts, uint32_t pc)
{
	return ((starts[pc >> 3] >> (pc & 7)) & 1);
}

static int	rt_try_ok(const t_dmethod *m, const t_dexcode *code, uint32_t i,
		const uint8_t *starts)
{
	const uint8_t	*t;
	t_dexhit		it;
	t_dexcatch		ca;
	uint32_t		start;

	if ((uint64_t)code->tries_off + 8 * ((uint64_t)i + 1) > m->dex->len)
		return (0);
	t = m->dex->p + code->tries_off + 8 * (size_t)i;
	start = (uint32_t)dexcode_rd32(t);
	if (start >= code->insns_size || !rt_start(starts, start)
		|| (uint32_t)(t[4] | (t[5] << 8)) > code->insns_size - start)
		return (0);
	if (dex_handler_open(m->dex, code, i, &it) != 0)
		return (0);
	while (dex_handler_next(&it, &ca) == 0)
	{
		if (ca.addr >= code->insns_size || !rt_start(starts, ca.addr))
			return (0);
	}
	return (1);
}

static void	rt_code_limits(const t_dex *d, const t_dexcode *code,
		t_dcodelimits *lim)
{
	lim->registers = code->registers;
	lim->ins = code->ins;
	lim->outs = code->outs;
	lim->strings = d->n[DEX_T_STRING];
	lim->types = d->n[DEX_T_TYPE];
	lim->fields = d->n[DEX_T_FIELD];
	lim->methods = d->n[DEX_T_METHOD];
	lim->protos = d->n[DEX_T_PROTO];
}

static int	rt_verify_one(t_dvm *vm, t_dclass *c, uint32_t i)
{
	const t_dex		*d;
	t_dcodelimits	lim;
	t_dexcode		code;
	uint32_t		t;

	d = &vm->classes->dex;
	if (c->methods[i].code_off == 0)
		return (0);
	if (dex_code(d, c->methods[i].code_off, &code) != 0)
		return (E_INVAL);
	rt_code_limits(d, &code, &lim);
	c->starts[i] = calloc(1, code.insns_size / 8 + 1);
	if (!c->starts[i])
		return (E_NOMEM);
	if (dexcode_verify(c->methods[i].insns, &lim, c->starts[i]) != 0)
		return (E_INVAL);
	t = 0;
	while (t < code.tries && rt_try_ok(&c->methods[i], &code, t, c->starts[i]))
		t++;
	if (t < code.tries)
		return (E_INVAL);
	return (0);
}

int	rt_verify(t_dvm *vm, t_dclass *c)
{
	uint32_t	i;
	int			r;

	i = 0;
	r = 0;
	while (r == 0 && i < c->nmethods)
		r = rt_verify_one(vm, c, i++);
	if (r == E_INVAL)
		r = rt_verr(vm, c);
	return (r);
}
