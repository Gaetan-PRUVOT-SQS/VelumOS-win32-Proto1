#ifndef FAKE_GFX_H
# define FAKE_GFX_H

# include <stdint.h>
# include "velum/gfx.h"

# define FAKE_PAPER 0xFFFFFFFFu
# define FAKE_INK 0xFF000000u

typedef struct s_fakegfx
{
	int32_t	calls;
	int32_t	outside;
	int32_t	repeated;
}	t_fakegfx;

extern t_fakegfx	g_fake;

void		fake_reset(void);
void		fake_surface(t_surface *s, uint32_t *px, int32_t w, int32_t h);
void		fake_alloc(t_surface *s, int32_t w, int32_t h);
uint64_t	fake_hash(const t_surface *s);
int			fake_write_ppm(const t_surface *s, const char *path, int scale);

#endif
