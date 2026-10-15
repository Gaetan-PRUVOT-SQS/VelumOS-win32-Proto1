#include "rsa_int.h"

static void	mod_fix(t_bn *r, const t_mod *m, uint32_t carry)
{
	if (carry || bn_cmp(r, &m->n, m->nw) >= 0)
		bn_sub(r, &m->n, m->nw);
}

void	bn_modmul(t_bn *r, const t_bn *a, const t_bn *b, const t_mod *m)
{
	t_bn		acc;
	uint32_t	i;

	i = 0;
	while (i < RSA_WORDS)
		acc.w[i++] = 0;
	i = m->nw * 32;
	while (i > 0)
	{
		i--;
		mod_fix(&acc, m, bn_shl1(&acc, m->nw));
		if ((a->w[i / 32] >> (i % 32)) & 1)
			mod_fix(&acc, m, bn_add(&acc, b, m->nw));
	}
	*r = acc;
}

void	bn_modexp(t_bn *r, const t_bn *base, uint32_t e, const t_mod *m)
{
	t_bn		acc;
	uint32_t	bit;

	bit = 0x80000000u;
	while (bit && !(e & bit))
		bit >>= 1;
	acc = *base;
	bit >>= 1;
	while (bit)
	{
		bn_modmul(&acc, &acc, &acc, m);
		if (e & bit)
			bn_modmul(&acc, &acc, base, m);
		bit >>= 1;
	}
	*r = acc;
}
