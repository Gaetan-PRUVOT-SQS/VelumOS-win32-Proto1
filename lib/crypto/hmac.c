#include "crypto_int.h"
#include "velum/libk.h"

static void	hmac_xor_block(uint8_t k[SHA256_BLOCK], uint8_t v)
{
	uint32_t	i;

	i = 0;
	while (i < SHA256_BLOCK)
	{
		k[i] ^= v;
		i++;
	}
}

void	hmac_prepare(t_hmac *h, const uint8_t *key, size_t keylen)
{
	uint8_t	k[SHA256_BLOCK];

	memset(k, 0, sizeof(k));
	if (key && keylen > SHA256_BLOCK)
		sha256(key, keylen, k);
	else if (key && keylen)
		memcpy(k, key, keylen);
	hmac_xor_block(k, 0x36);
	sha256_init(&h->inner);
	sha256_update(&h->inner, k, sizeof(k));
	hmac_xor_block(k, 0x36 ^ 0x5c);
	sha256_init(&h->outer);
	sha256_update(&h->outer, k, sizeof(k));
	secure_zero(k, sizeof(k));
}

void	hmac_update(t_hmac *h, const void *data, size_t n)
{
	sha256_update(&h->inner, data, n);
}

void	hmac_final(t_hmac *h, uint8_t out[SHA256_LEN])
{
	uint8_t	digest[SHA256_LEN];

	sha256_final(&h->inner, digest);
	sha256_update(&h->outer, digest, sizeof(digest));
	sha256_final(&h->outer, out);
	secure_zero(digest, sizeof(digest));
}

void	hmac_sha256(const t_pbkdf2 *k, const void *m, size_t n, uint8_t *o)
{
	t_hmac	h;

	if (k)
		hmac_prepare(&h, k->pw, k->pwlen);
	else
		hmac_prepare(&h, NULL, 0);
	hmac_update(&h, m, n);
	hmac_final(&h, o);
}
