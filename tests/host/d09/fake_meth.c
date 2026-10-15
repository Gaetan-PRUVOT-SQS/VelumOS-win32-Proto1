#include "d09.h"

static int	fk_fill(t_dmethod *m, const t_dexmember *mb)
{
	t_dexcode	c;

	m->dex = &g_fk.dex;
	m->access = mb->access;
	m->code_off = mb->code_off;
	if (mb->code_off == 0 || dex_code(&g_fk.dex, mb->code_off, &c) < 0)
		return (0);
	m->insns = (t_span){c.insns, (size_t)c.insns_size * 2};
	m->registers = (uint16_t)c.registers;
	m->ins = (uint16_t)c.ins;
	m->outs = (uint16_t)c.outs;
	m->tries = (uint16_t)c.tries;
	return (0);
}

static int	fk_native(uint32_t idx)
{
	if (!g_fk.native)
		return (-1);
	g_fk.meth[idx].native = g_fk.native;
	g_fk.meth[idx].ins = g_fk.native_ins;
	return (0);
}

static int	fk_bind(uint32_t idx)
{
	t_dexmethod	id;
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	mb;
	int			def;

	if (dex_method(&g_fk.dex, idx, &id) < 0)
		return (-1);
	def = dex_find_class(&g_fk.dex, fk_type(id.class_idx));
	if (def < 0)
		return (fk_native(idx));
	if (dex_class(&g_fk.dex, (uint32_t)def, &c) < 0
		|| dex_cdata_open(&g_fk.dex, &c, &it) < 0)
		return (-1);
	while (dex_cdata_next(&it, &mb) == 0)
		if (mb.kind >= DEX_M_DIRECT && mb.idx == idx)
			return (fk_fill(&g_fk.meth[idx], &mb));
	return (-1);
}

int	dvmrt_resolve(t_dvm *vm, const t_dmethod *from, t_dinvoke *inv)
{
	uint32_t	idx;

	(void)from;
	idx = inv->method_idx;
	if (idx >= FK_METHODS)
		return (E_RANGE);
	if (!g_fk.known[idx] && fk_bind(idx) < 0)
		return (dvm_throw(vm, "Ljava/lang/NoSuchMethodError;", NULL));
	g_fk.known[idx] = 1;
	if (inv->kind != DIK_STATIC && inv->self == DVM_NULL)
		return (dvm_throw(vm, FK_NPE, NULL));
	inv->target = &g_fk.meth[idx];
	return (0);
}

const t_dmethod	*fk_method(const char *cls, const char *name)
{
	t_dexmethod	id;
	t_dexstr	s;
	t_dinvoke	inv;
	uint32_t	idx;

	idx = 0;
	while (idx < g_fk.dex.n[DEX_T_METHOD] && idx < FK_METHODS)
	{
		inv = (t_dinvoke){idx, DVM_NULL, NULL, DIK_STATIC};
		if (dex_method(&g_fk.dex, idx, &id) == 0
			&& dex_string(&g_fk.dex, id.name_idx, &s) == 0
			&& fk_same(s.p, name) && fk_same(fk_type(id.class_idx), cls)
			&& dvmrt_resolve(&g_fk.vm, NULL, &inv) == 0)
			return (inv.target);
		idx++;
	}
	return (NULL);
}
