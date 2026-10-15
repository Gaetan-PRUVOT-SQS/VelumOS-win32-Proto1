#include "rsa_int.h"

void	bn_from_be(t_bn *r, const uint8_t *p, size_t len)
{
	size_t	i;

	i = 0;
	while (i < RSA_WORDS)
		r->w[i++] = 0;
	i = 0;
	while (i < len && i < RSA_MAX_BYTES)
	{
		r->w[i / 4] |= (uint32_t)p[len - 1 - i] << (8 * (i % 4));
		i++;
	}
}

void	bn_to_be(const t_bn *a, uint8_t *p, size_t len)
{
	size_t	i;

	i = 0;
	while (i < len && i < RSA_MAX_BYTES)
	{
		p[len - 1 - i] = (uint8_t)(a->w[i / 4] >> (8 * (i % 4)));
		i++;
	}
}
