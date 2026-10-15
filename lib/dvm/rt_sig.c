#include "rt_int.h"

uint16_t	rt_sig_words(const char *s)
{
	uint16_t	n;
	uint16_t	wide;

	n = 0;
	if (*s == '(')
		s++;
	while (*s && *s != ')')
	{
		wide = (*s == 'J' || *s == 'D');
		while (*s == '[')
			s++;
		if (*s == 'L')
			while (*s && *s != ';')
				s++;
		if (*s)
			s++;
		n += 1 + wide;
	}
	return (n);
}

static size_t	rt_sig_one(const t_dex *d, uint32_t type, char *out, size_t n)
{
	t_dexstr	s;
	uint32_t	k;

	if (dex_type(d, type, &s) != 0)
		return (n);
	k = 0;
	while (s.p[k])
	{
		if (out)
			out[n] = s.p[k];
		n++;
		k++;
	}
	return (n);
}

static size_t	rt_sig_put(const t_dex *d, const t_dexproto *p, char *out)
{
	size_t		n;
	uint32_t	i;

	n = 1;
	if (out)
		out[0] = '(';
	i = 0;
	while (i < p->params.n)
		n = rt_sig_one(d, dex_list_at(&p->params, i++), out, n);
	if (out)
		out[n] = ')';
	n = rt_sig_one(d, p->return_idx, out, n + 1);
	if (out)
		out[n] = '\0';
	return (n);
}

char	*rt_sig_build(const t_dex *d, const t_dexproto *p)
{
	char	*sig;

	sig = calloc(1, rt_sig_put(d, p, NULL) + 1);
	if (sig)
		rt_sig_put(d, p, sig);
	return (sig);
}

int	rt_verr(t_dvm *vm, const t_dclass *c)
{
	return (dvm_throw(vm, DX_VERIFY, c->desc));
}
