#ifndef LAYOUT_H
# define LAYOUT_H

# include <stdint.h>
# include "velum/gfx.h"

t_rect	lay_rect(int32_t x, int32_t y, int32_t w, int32_t h);
int		lay_inside(t_rect outer, t_rect r);
int		lay_overlap(t_rect a, t_rect b);
int		lay_contains(t_rect r, int32_t x, int32_t y);
int32_t	lay_clamp(int32_t v, int32_t lo, int32_t hi);

#endif
