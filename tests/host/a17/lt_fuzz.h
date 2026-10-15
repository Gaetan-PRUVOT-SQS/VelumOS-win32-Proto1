#ifndef LT_FUZZ_H
# define LT_FUZZ_H

# include <stdbool.h>
# include <stdint.h>
# include "lt.h"
# include "luna_int.h"

uint32_t	ltf_rnd(uint64_t *seed, uint32_t n);
t_rect		ltf_zone(t_rect r, t_rect clip, int32_t w, int32_t h);
bool		ltf_surface_ok(const t_lt *t, t_rect zone, t_color back);
void		ltf_case(uint64_t *seed, int32_t *c);

#endif
