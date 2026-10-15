#ifndef RSA_INT_H
# define RSA_INT_H

# include "velum/apk/rsa.h"
# include "velum/err.h"

# define DER_INT 0x02
# define DER_BITS 0x03
# define DER_NULL 0x05
# define DER_OID 0x06
# define DER_SEQ 0x30
# define DER_CTX0 0xa0

typedef struct s_der
{
	t_span	all;
	t_span	val;
	t_span	rest;
	uint8_t	tag;
}	t_der;

typedef struct s_bn
{
	uint32_t	w[RSA_WORDS];
}	t_bn;

typedef struct s_mod
{
	t_bn		n;
	uint32_t	nw;
}	t_mod;

int			der_read(t_span in, t_der *out);
int			der_take(t_span in, uint8_t tag, t_der *out);
int			der_only(t_span in, uint8_t tag, t_span *val);
int			der_uint(t_span in, t_span *mag, t_span *rest);
int			rsa_key_check(const t_rsapub *k);
void		bn_from_be(t_bn *r, const uint8_t *p, size_t len);
void		bn_to_be(const t_bn *a, uint8_t *p, size_t len);
int			bn_cmp(const t_bn *a, const t_bn *b, uint32_t nw);
uint32_t	bn_add(t_bn *r, const t_bn *b, uint32_t nw);
uint32_t	bn_sub(t_bn *r, const t_bn *b, uint32_t nw);
uint32_t	bn_shl1(t_bn *r, uint32_t nw);
void		bn_modmul(t_bn *r, const t_bn *a, const t_bn *b, const t_mod *m);
void		bn_modexp(t_bn *r, const t_bn *base, uint32_t e, const t_mod *m);

#endif
