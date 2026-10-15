#ifndef FAKE_GFX_H
# define FAKE_GFX_H

# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"

t_rect	fk_rect(int64_t x0, int64_t y0, int64_t x1, int64_t y1);
t_rect	fk_cut(t_rect a, t_rect b);
t_rect	fk_visible(const t_surface *s, t_rect r);

#endif
