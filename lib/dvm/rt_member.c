#include "rt_int.h"

static int	rt_count(const t_dex *d, const t_dexclass *dc, t_dclass *c)
{
	t_dexcdata	it;
	t_dexmember	m;
	uint32_t	nf;
	uint32_t	nm;

	nf = 0;
	nm = 0;
	if (dc->class_data_off != 0 && dex_cdata_open(d, dc, &it) != 0)
		return (E_INVAL);
	while (dc->class_data_off != 0 && dex_cdata_next(&it, &m) == 0)
	{
		nf += (m.kind < DEX_M_DIRECT);
		nm += (m.kind >= DEX_M_DIRECT);
	}
	if (nf > DMEMBERS_MAX || nm > DMEMBERS_MAX)
		return (E_INVAL);
	c->nfields = nf;
	c->nmethods = nm;
	return (0);
}

static int	rt_members_alloc(t_dclass *c)
{
	c->fields = calloc(c->nfields + 1, sizeof(t_dfield));
	c->statics = calloc(2 * c->nfields + 1, sizeof(uint32_t));
	c->methods = calloc(c->nmethods + 1, sizeof(t_dmethod));
	c->sigs = calloc(c->nmethods + 1, sizeof(char *));
	c->starts = calloc(c->nmethods + 1, sizeof(uint8_t *));
	if (!c->fields || !c->methods || !c->sigs || !c->starts || !c->statics)
		return (E_NOMEM);
	return (0);
}

static void	rt_field_fill(const t_dex *d, const t_dexmember *m, t_dclass *c,
		t_dfield *f)
{
	t_dexfield	df;

	if (dex_field(d, m->idx, &df) != 0)
		return ;
	f->type = rt_dtype(d, df.type_idx);
	if (!f->type)
		return ;
	f->name = rt_dstr(d, df.name_idx);
	f->wide = (f->type[0] == 'J' || f->type[0] == 'D');
	f->is_ref = (f->type[0] == 'L' || f->type[0] == '[');
	f->is_static = (m->kind == DEX_M_SFIELD);
	if (f->is_static)
	{
		f->slot = c->nstatics;
		c->nstatics += 1 + f->wide;
		return ;
	}
	f->slot = c->words;
	c->words += 1 + f->wide;
}

static int	rt_members_fill(t_dvm *vm, const t_dexclass *dc, t_dclass *c)
{
	t_dexcdata	it;
	t_dexmember	m;
	uint32_t	fi;
	uint32_t	mi;
	int			r;

	fi = 0;
	mi = 0;
	r = 0;
	if (dc->class_data_off == 0
		|| dex_cdata_open(&vm->classes->dex, dc, &it) != 0)
		return (0);
	while (r == 0 && dex_cdata_next(&it, &m) == 0)
	{
		if (m.kind < DEX_M_DIRECT && fi < c->nfields)
			rt_field_fill(&vm->classes->dex, &m, c, &c->fields[fi++]);
		else if (m.kind >= DEX_M_DIRECT && mi < c->nmethods)
			r = rt_method_fill(vm, &m, c, mi++);
	}
	return (r);
}

int	rt_members(t_dvm *vm, const t_dexclass *dc, t_dclass *c)
{
	int	r;

	r = rt_count(&vm->classes->dex, dc, c);
	if (r == 0)
		r = rt_members_alloc(c);
	if (r == 0)
		r = rt_members_fill(vm, dc, c);
	if (r == 0 && c->words > DWORDS_MAX)
		r = E_INVAL;
	if (r == 0)
		r = rt_statics_init(vm, dc, c);
	if (r == E_INVAL)
		r = rt_verr(vm, c);
	return (r);
}
