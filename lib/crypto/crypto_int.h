#ifndef CRYPTO_INT_H
# define CRYPTO_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/crypto.h"

# define PBKDF2_ITERS_MAX 10000000
# define PBKDF2_BLOCKS_MAX 0xffffffffull

typedef struct s_hmac
{
	t_sha256	inner;
	t_sha256	outer;
}	t_hmac;

void		sha256_compress(uint32_t h[8], const uint8_t block[SHA256_BLOCK]);
uint32_t	crypto_load_be32(const uint8_t *p);
void		crypto_store_be32(uint8_t *p, uint32_t v);
void		crypto_store_be64(uint8_t *p, uint64_t v);
uint32_t	crypto_load_le32(const uint8_t *p);
void		crypto_store_le32(uint8_t *p, uint32_t v);
void		hmac_prepare(t_hmac *h, const uint8_t *key, size_t keylen);
void		hmac_update(t_hmac *h, const void *data, size_t n);
void		hmac_final(t_hmac *h, uint8_t out[SHA256_LEN]);
int			pbkdf2_validate(const t_pbkdf2 *p, size_t outlen);
void		chacha20_double_round(uint32_t x[16]);

#endif
