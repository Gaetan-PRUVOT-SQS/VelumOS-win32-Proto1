#ifndef DEX_INT_H
# define DEX_INT_H

# include "velum/apk/dex.h"
# include "velum/err.h"

typedef struct s_dexcur
{
	const uint8_t	*p;
	size_t			len;
	size_t			pos;
	int				err;
}	t_dexcur;

uint32_t		dex_u16(const uint8_t *p);
uint32_t		dex_u32(const uint8_t *p);
int				dex_fits(const t_dex *d, uint64_t off, uint64_t n,
					uint64_t size);
t_dexcur		dex_cur(const t_dex *d, uint64_t off);
uint32_t		dex_byte(t_dexcur *c);
uint32_t		dex_uleb(t_dexcur *c);
int32_t			dex_sleb(t_dexcur *c);
uint32_t		dex_ulebp1(t_dexcur *c);
const uint8_t	*dex_item(const t_dex *d, uint32_t table, uint32_t i);
int				dex_mutf8_check(t_span s, uint32_t *size, uint32_t *units);
int				dex_verify_ids(const t_dex *d);
int				dex_verify_classes(const t_dex *d);

#endif
