#include "rsa_int.h"

int	bn_cmp(const t_bn *a, const t_bn *b, uint32_t nw)
{
	while (nw > 0)
	{
		nw--;
		if (a->w[nw] > b->w[nw])
			return (1);
		if (a->w[nw] < b->w[nw])
			return (-1);
	}
	return (0);
}

uint32_t	bn_add(t_bn *r, const t_bn *b, uint32_t nw)
{
	uint64_t	c;
	uint32_t	i;

	c = 0;
	i = 0;
	while (i < nw)
	{
		c += (uint64_t)r->w[i] + b->w[i];
		r->w[i] = (uint32_t)c;
		c >>= 32;
		i++;
	}
	return ((uint32_t)c);
}

uint32_t	bn_sub(t_bn *r, const t_bn *b, uint32_t nw)
{
	uint64_t	d;
	uint32_t	borrow;
	uint32_t	i;

	borrow = 0;
	i = 0;
	while (i < nw)
	{
		d = (uint64_t)r->w[i] - b->w[i] - borrow;
		r->w[i] = (uint32_t)d;
		borrow = (uint32_t)(d >> 63);
		i++;
	}
	return (borrow);
}

uint32_t	bn_shl1(t_bn *r, uint32_t nw)
{
	uint32_t	c;
	uint32_t	t;
	uint32_t	i;

	c = 0;
	i = 0;
	while (i < nw)
	{
		t = r->w[i] >> 31;
		r->w[i] = (r->w[i] << 1) | c;
		c = t;
		i++;
	}
	return (c);
}
