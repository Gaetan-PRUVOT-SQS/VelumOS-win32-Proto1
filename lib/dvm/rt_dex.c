#include "rt_int.h"

const char	*rt_dstr(const t_dex *d, uint32_t idx)
{
	t_dexstr	s;

	if (dex_string(d, idx, &s) != 0)
		return (NULL);
	return (s.p);
}

const char	*rt_dtype(const t_dex *d, uint32_t idx)
{
	t_dexstr	s;

	if (dex_type(d, idx, &s) != 0)
		return (NULL);
	return (s.p);
}

int	dvm_load_dex(t_dvm *vm, t_span dex)
{
	struct s_dclasses	*cs;
	int					r;

	if (!vm || !dex.p)
		return (E_INVAL);
	cs = vm->classes;
	if (cs->has_dex)
		return (E_NOTSUP);
	r = dex_open(&cs->dex, dex);
	if (r != 0)
		return (r);
	cs->def_state = calloc(cs->dex.n[DEX_T_CLASS] + 1, 1);
	cs->interned = calloc(cs->dex.n[DEX_T_STRING] + 1, sizeof(t_dref));
	if (cs->def_state && cs->interned)
		cs->has_dex = 1;
	if (cs->has_dex)
		return (0);
	free(cs->def_state);
	free(cs->interned);
	cs->def_state = NULL;
	cs->interned = NULL;
	return (E_NOMEM);
}

static int	rt_native_fill(const t_dvm *vm, const t_dnative *t, t_dmethod *m)
{
	if (!t->cls || !t->name || !t->sig || !t->fn)
		return (E_INVAL);
	m->cls = rt_find(vm, t->cls);
	if (!m->cls)
		return (E_NOENT);
	m->name = t->name;
	m->sig = t->sig;
	m->native = t->fn;
	m->access = t->access | ACC_NATIVE;
	m->ins = rt_sig_words(t->sig) + !(t->access & ACC_STATIC);
	return (0);
}

int	dvm_natives(t_dvm *vm, const t_dnative *tab, uint32_t n)
{
	struct s_dclasses	*cs;
	t_dmethod			*m;
	uint32_t			i;

	if (!vm || !tab || n == 0)
		return (E_INVAL);
	cs = vm->classes;
	if (cs->nnat >= DNAT_TABS)
		return (E_RANGE);
	m = calloc(n, sizeof(*m));
	if (!m)
		return (E_NOMEM);
	i = 0;
	while (i < n && rt_native_fill(vm, &tab[i], &m[i]) == 0)
		i++;
	if (i < n)
	{
		free(m);
		return (E_NOENT);
	}
	cs->natm[cs->nnat] = m;
	cs->natn[cs->nnat++] = n;
	return (0);
}
