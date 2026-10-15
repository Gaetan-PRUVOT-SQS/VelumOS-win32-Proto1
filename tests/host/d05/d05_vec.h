#ifndef D05_VEC_H
# define D05_VEC_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/rsa.h"

# define VEC_CERT 1
# define VEC_PARSE 2
# define VEC_OK 4
# define VEC_BASE_CERT "pos-2048-e3-cert"
# define VEC_BASE_SPKI "pos-2048-e3-spki"

typedef struct s_vec
{
	const char		*name;
	const uint8_t	*key;
	size_t			key_len;
	const uint8_t	*sig;
	size_t			sig_len;
	const uint8_t	*msg;
	size_t			msg_len;
	const uint8_t	*digest;
	uint32_t		flags;
}	t_vec;

extern const t_vec	g_vec[];
extern const size_t	g_vec_count;

const t_vec	*vec_find(const char *name);
int			key_from(t_span src, int cert, t_rsapub *k);
int			sig_run(const t_rsapub *k, t_span sig, const uint8_t *digest);
int			vec_check(const t_vec *v);

#endif
