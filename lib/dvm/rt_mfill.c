#include "rt_int.h"

static int	rt_method_code(const t_dex *d, t_dmethod *m)
{
	t_dexcode	code;

	if (m->code_off == 0)
		return (0);
	if (dex_code(d, m->code_off, &code) != 0)
		return (E_INVAL);
	if (code.ins != m->ins || code.ins > code.registers
		|| code.registers > 0xffff || code.outs > 0xffff
		|| code.tries > 0xffff)
		return (E_INVAL);
	m->registers = (uint16_t)code.registers;
	m->outs = (uint16_t)code.outs;
	m->tries = (uint16_t)code.tries;
	m->insns.p = code.insns;
	m->insns.len = (size_t)code.insns_size * 2;
	return (0);
}

int	rt_method_fill(t_dvm *vm, const t_dexmember *mb, t_dclass *c, uint32_t i)
{
	const t_dex	*d;
	t_dexmethod	dm;
	t_dexproto	p;
	t_dmethod	*m;

	d = &vm->classes->dex;
	m = &c->methods[i];
	if (dex_method(d, mb->idx, &dm) != 0
		|| dex_proto(d, dm.proto_idx, &p) != 0)
		return (E_INVAL);
	c->sigs[i] = rt_sig_build(d, &p);
	if (!c->sigs[i])
		return (E_NOMEM);
	m->cls = c;
	m->name = rt_dstr(d, dm.name_idx);
	m->shorty = rt_dstr(d, p.shorty_idx);
	m->sig = c->sigs[i];
	m->dex = d;
	m->access = mb->access;
	m->code_off = mb->code_off;
	m->slot = i;
	m->ins = rt_sig_words(m->sig) + !(mb->access & ACC_STATIC);
	if (!m->name)
		return (E_INVAL);
	return (rt_method_code(d, m));
}

int	dvm_method_start(const t_dmethod *m, uint32_t pc)
{
	const uint8_t	*starts;

	if (!m || !m->cls || !m->cls->starts || m->native
		|| m->slot >= m->cls->nmethods || pc >= m->insns.len / 2)
		return (0);
	starts = m->cls->starts[m->slot];
	if (!starts)
		return (0);
	return ((starts[pc >> 3] >> (pc & 7)) & 1);
}
