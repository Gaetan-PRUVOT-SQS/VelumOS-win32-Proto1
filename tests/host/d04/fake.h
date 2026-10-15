#ifndef FAKE_H
# define FAKE_H

# include <stdint.h>
# include "velum/apk/dexcode.h"
# include "velum/err.h"

typedef struct s_code
{
	uint8_t		b[256];
	uint32_t	n;
	uint8_t		starts[16];
	uint32_t	accepted;
}	t_code;

extern t_code	g_c;

void			fake_reset(void);
void			fake_u(uint32_t unit);
void			fake_us(const uint16_t *u, uint32_t n);
t_dinsn			fake_dec(uint64_t lo, uint32_t hi);
int				fake_dec_r(uint64_t lo, uint32_t hi);
t_dcodelimits	fake_lim(uint32_t regs);
int				fake_v(uint32_t regs);
int				fake_exact(const uint8_t *src, uint32_t units,
					const t_dcodelimits *lim);
int				fake_property(t_span s, const t_dcodelimits *lim,
					const uint8_t *st);
uint32_t		fake_sample(int i);

#endif
