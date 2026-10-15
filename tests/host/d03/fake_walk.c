#include "fake.h"
#include "dex_int.h"

static uint32_t	walk_code(const t_dex *d, uint32_t off)
{
	t_dexcode	code;
	t_dexhit	it;
	t_dexcatch	c;
	uint32_t	i;
	uint32_t	n;

	n = 0;
	if (off == 0 || dex_code(d, off, &code) != E_OK)
		return (0);
	if (code.insns_size > 0)
		n += code.insns[0] + code.insns[2 * (size_t)code.insns_size - 1];
	dex_try_find(d, &code, off & 7);
	i = 0;
	while (i < code.tries)
	{
		if (dex_handler_open(d, &code, i, &it) == E_OK)
			while (dex_handler_next(&it, &c) == E_OK)
				n++;
		i++;
	}
	return (n);
}

static uint32_t	walk_class(const t_dex *d, uint32_t i)
{
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	m;
	t_dexvalue	v;
	uint32_t	n;

	n = 0;
	if (dex_class(d, i, &c) != E_OK || dex_cdata_open(d, &c, &it) != E_OK)
		return (0);
	while (dex_cdata_next(&it, &m) == E_OK)
		n += 1 + walk_code(d, m.code_off);
	i = 0;
	while (i < 16)
		n += (dex_static_value(d, &c, i++, &v) == E_OK);
	return (n);
}

static uint32_t	walk_ids(const t_dex *d)
{
	t_dexstr	s;
	t_dexproto	p;
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < d->n[DEX_T_STRING])
		if (dex_string(d, i++, &s) == E_OK)
			n += (s.p[s.size] == 0) + (uint8_t)s.p[0];
	i = 0;
	while (i < d->n[DEX_T_TYPE])
		n += (dex_type(d, i++, &s) == E_OK);
	i = 0;
	while (i < d->n[DEX_T_PROTO])
		if (dex_proto(d, i++, &p) == E_OK)
			n += (dex_list_at(&p.params, p.params.n - 1) != DEX_NO_INDEX);
	return (n);
}

static uint32_t	walk_refs(const t_dex *d)
{
	t_dexfield	f;
	t_dexmethod	m;
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < d->n[DEX_T_FIELD])
		n += (dex_field(d, i++, &f) == E_OK);
	i = 0;
	while (i < d->n[DEX_T_METHOD])
		n += (dex_method(d, i++, &m) == E_OK);
	return (n);
}

uint32_t	fake_walk(const t_dex *d)
{
	uint32_t	i;
	uint32_t	n;

	n = walk_ids(d) + walk_refs(d);
	i = 0;
	while (i < d->n[DEX_T_CLASS])
		n += walk_class(d, i++);
	dex_find_class(d, "Lvelum/Derived;");
	return (n);
}
