#include "gfx_int.h"

t_color	gfx_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return (((t_color)a << 24) | ((t_color)r << 16) | ((t_color)g << 8) | b);
}

t_color	gfx_blend(t_color dst, t_color src)
{
	return (gfx_blend_px(dst, src));
}

static uint32_t	lerp_lanes(uint32_t a, uint32_t b, uint32_t t)
{
	uint32_t	v;

	v = a * (256 - t) + b * t + 0x00800080;
	return ((v >> 8) & 0x00ff00ff);
}

t_color	gfx_lerp(t_color a, t_color b, uint32_t t256)
{
	uint32_t	ag;
	uint32_t	rb;

	if (t256 > 256)
		t256 = 256;
	ag = lerp_lanes((a >> 8) & 0x00ff00ff, (b >> 8) & 0x00ff00ff, t256);
	rb = lerp_lanes(a & 0x00ff00ff, b & 0x00ff00ff, t256);
	return ((ag << 8) | rb);
}
