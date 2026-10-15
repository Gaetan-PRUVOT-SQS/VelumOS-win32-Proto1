#include "crypto_int.h"
#include "velum/libk.h"

static void	sha256_pad(t_sha256 *c)
{
	uint64_t	bits;

	bits = c->total << 3;
	c->buf[c->fill++] = 0x80;
	if (c->fill > SHA256_BLOCK - 8)
	{
		memset(c->buf + c->fill, 0, SHA256_BLOCK - c->fill);
		sha256_compress(c->h, c->buf);
		c->fill = 0;
	}
	memset(c->buf + c->fill, 0, SHA256_BLOCK - 8 - c->fill);
	crypto_store_be64(c->buf + SHA256_BLOCK - 8, bits);
	sha256_compress(c->h, c->buf);
}

void	sha256_final(t_sha256 *c, uint8_t out[SHA256_LEN])
{
	uint32_t	i;

	sha256_pad(c);
	i = 0;
	while (i < 8)
	{
		crypto_store_be32(out + i * 4, c->h[i]);
		i++;
	}
	secure_zero(c, sizeof(*c));
}

void	sha256(const void *data, size_t n, uint8_t out[SHA256_LEN])
{
	t_sha256	c;

	sha256_init(&c);
	sha256_update(&c, data, n);
	sha256_final(&c, out);
}
