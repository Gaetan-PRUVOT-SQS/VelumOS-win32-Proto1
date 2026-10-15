#ifndef CRYPTO_H
# define CRYPTO_H

# include <stddef.h>
# include <stdint.h>

# define SHA256_LEN 32
# define SHA256_BLOCK 64

typedef struct s_sha256
{
	uint32_t	h[8];
	uint64_t	total;
	uint8_t		buf[SHA256_BLOCK];
	uint32_t	fill;
}	t_sha256;

typedef struct s_pbkdf2
{
	const uint8_t	*pw;
	size_t			pwlen;
	const uint8_t	*salt;
	size_t			saltlen;
	uint32_t		iters;
}	t_pbkdf2;

void	sha256_init(t_sha256 *c);
void	sha256_update(t_sha256 *c, const void *data, size_t n);
void	sha256_final(t_sha256 *c, uint8_t out[SHA256_LEN]);
void	sha256(const void *data, size_t n, uint8_t out[SHA256_LEN]);
void	hmac_sha256(const t_pbkdf2 *k, const void *m, size_t n, uint8_t *o);
int		pbkdf2_sha256(const t_pbkdf2 *p, uint8_t *out, size_t outlen);
int		ct_equal(const void *a, const void *b, size_t n);
void	secure_zero(void *p, size_t n);

#endif
