#include "rt_int.h"

static int	rt_intern_new(t_dvm *vm, const char *s, t_dref *out)
{
	t_text	t;
	int		r;

	t.cap = strlen(s) + 1;
	t.p = calloc(1, t.cap);
	if (!t.p)
		return (E_NOMEM);
	r = mutf8_to_utf8(s, t);
	if (r >= 0)
		r = dvm_string_utf8_new(vm, t.p, out);
	free(t.p);
	return (r);
}

int	rt_intern(t_dvm *vm, uint32_t idx, t_dref *out)
{
	struct s_dclasses	*cs;
	const char			*s;
	int					r;

	cs = vm->classes;
	*out = DVM_NULL;
	s = NULL;
	if (cs->has_dex)
		s = rt_dstr(&cs->dex, idx);
	if (!s)
		return (E_INVAL);
	*out = cs->interned[idx];
	if (rt_obj(vm, *out, DOK_STRING))
		return (0);
	r = rt_intern_new(vm, s, out);
	if (r == 0)
		cs->interned[idx] = *out;
	return (r);
}

static int	rt_static_set(t_dvm *vm, t_dclass *c, const t_dfield *f,
		const t_dexvalue *v)
{
	t_dref	s;
	int		r;

	if (v->kind == DEX_V_NULL)
		return (0);
	if (v->kind == DEX_V_STRING)
	{
		if (!f->is_ref)
			return (E_INVAL);
		r = rt_intern(vm, (uint32_t)v->i, &s);
		c->statics[f->slot] = s;
		return (r);
	}
	if (f->is_ref)
		return (E_INVAL);
	c->statics[f->slot] = (uint32_t)v->i;
	if (f->wide)
		c->statics[f->slot + 1] = (uint32_t)((uint64_t)v->i >> 32);
	return (0);
}

int	rt_statics_init(t_dvm *vm, const t_dexclass *dc, t_dclass *c)
{
	t_dexvalue	v;
	uint32_t	i;
	uint32_t	ord;
	int			r;

	i = 0;
	ord = 0;
	r = 0;
	while (r == 0 && i < c->nfields)
	{
		if (c->fields[i].type && c->fields[i].is_static)
		{
			if (dex_static_value(&vm->classes->dex, dc, ord, &v) == 0)
				r = rt_static_set(vm, c, &c->fields[i], &v);
			ord++;
		}
		i++;
	}
	return (r);
}
