#ifndef GFX_PX_H
# define GFX_PX_H

# include "velum/gfx.h"

static inline uint32_t	gfx_mix_lanes(uint32_t s, uint32_t d, uint32_t a)
{
	uint32_t	t;

	t = s * a + d * (255 - a) + 0x00800080;
	t = t + ((t >> 8) & 0x00ff00ff);
	return ((t >> 8) & 0x00ff00ff);
}

static inline t_color	gfx_blend_px(t_color dst, t_color src)
{
	uint32_t	a;
	uint32_t	ag;
	uint32_t	rb;

	a = src >> 24;
	if (a == 255)
		return (src);
	if (a == 0)
		return (dst);
	src |= 0xff000000;
	ag = gfx_mix_lanes((src >> 8) & 0x00ff00ff, (dst >> 8) & 0x00ff00ff, a);
	rb = gfx_mix_lanes(src & 0x00ff00ff, dst & 0x00ff00ff, a);
	return ((ag << 8) | rb);
}

static inline void	gfx_store(uint32_t *p, t_color c)
{
	if ((c >> 24) == 255)
		*p = c;
	else if ((c >> 24) != 0)
		*p = gfx_blend_px(*p, c);
}

#endif
