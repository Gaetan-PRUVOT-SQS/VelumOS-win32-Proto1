#ifndef RSA_H
# define RSA_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apkdef.h"

# define RSA_MIN_BITS 2048
# define RSA_MAX_BITS 4096
# define RSA_MIN_BYTES 256
# define RSA_MAX_BYTES 512
# define RSA_WORDS 128
# define RSA_MIN_EXP 3
# define RSA_DIGEST_LEN 32

typedef struct s_rsapub
{
	uint8_t		n[RSA_MAX_BYTES];
	uint32_t	nlen;
	uint32_t	e;
}	t_rsapub;

int	x509_spki(t_span cert, t_span *spki);
int	der_spki_rsa(t_span spki, t_rsapub *out);
int	rsa_pkcs1_sha256_verify(const t_rsapub *k, t_span sig,
		const uint8_t digest[32]);

#endif
