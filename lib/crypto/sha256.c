#include "crypto_int.h"
#include "velum/libk.h"
#include "velum/util.h"

void	sha256_init(t_sha256 *c)
{
	c->h[0] = 0x6a09e667;
	c->h[1] = 0x0bb67ae85;
	c->h[2] = 0x3c6ef372;
	c->h[3] = 0xa54ff53a;
	c->h[4] = 0x510e527f;
	c->h[5] = 0x9b05688c;
	c->h[6] = 0x1f83d9ab;
	c->h[7] = 0x5be0cd19;
	c->total = 0;
	c->fill = 0;
}

static size_t	sha256_topup(t_sha256 *c, const uint8_t *p, size_t n)
{
	size_t	take;

	if (!c->fill)
		return (0);
	take = min_u64(SHA256_BLOCK - c->fill, n);
	memcpy(c->buf + c->fill, p, take);
	c->fill += (uint32_t)take;
	if (c->fill == SHA256_BLOCK)
	{
		sha256_compress(c->h, c->buf);
		c->fill = 0;
	}
	return (take);
}

void	sha256_update(t_sha256 *c, const void *data, size_t n)
{
	const uint8_t	*p;
	size_t			take;

	if (!n)
		return ;
	p = data;
	c->total += n;
	take = sha256_topup(c, p, n);
	p += take;
	n -= take;
	while (n >= SHA256_BLOCK)
	{
		sha256_compress(c->h, p);
		p += SHA256_BLOCK;
		n -= SHA256_BLOCK;
	}
	if (n)
	{
		memcpy(c->buf, p, n);
		c->fill = (uint32_t)n;
	}
}
