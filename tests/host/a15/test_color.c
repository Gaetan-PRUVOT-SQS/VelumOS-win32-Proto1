#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static uint32_t	blend_row(uint32_t a, uint32_t s)
{
	uint32_t	d;
	uint32_t	bad;
	t_color		src;
	t_color		dst;

	bad = 0;
	d = 0;
	while (d < 256)
	{
		src = ref_spread(a, s, 5);
		dst = ref_spread((d * 5) & 255, d, 11);
		if (gfx_blend(dst, src) != ref_blend(dst, src))
			bad++;
		d++;
	}
	return (bad);
}

static void	blend_exhaustif(void)
{
	uint32_t	a;
	uint32_t	s;

	a = 0;
	while (a < 256)
	{
		s = 0;
		while (s < 256)
		{
			h_eq_u64("blend: ecarts de la ligne (a, s)", blend_row(a, s), 0);
			s++;
		}
		a++;
	}
}

static void	blend_bornes(void)
{
	uint32_t	d;

	h_eq_u64("opaque", gfx_blend(0xff102030, 0xffabcdef), 0xffabcdef);
	h_eq_u64("transparent", gfx_blend(0x80102030, 0x00abcdef), 0x80102030);
	h_eq_u64("noir sur blanc", gfx_blend(0xffffffff, 0x80000000), 0xff7f7f7f);
	h_eq_u64("blanc sur noir", gfx_blend(0xff000000, 0x80ffffff), 0xff808080);
	h_eq_u64("alpha 128 sur 0", gfx_blend(0, 0x80ff0000) >> 24, 128);
	d = 0;
	while (d < 256)
	{
		h_eq_u64("alpha dst gardee", gfx_blend(d << 24, 0x00ffffff) >> 24, d);
		h_eq_u64("alpha dst opaque", gfx_blend(0xff000000, d << 24) >> 24, 255);
		d++;
	}
}

static void	lerp_formule(void)
{
	uint32_t	t;
	uint32_t	k;
	t_color		a;
	t_color		b;

	k = 0;
	while (k < 2000)
	{
		a = (t_color)rng_next();
		b = (t_color)rng_next();
		t = (uint32_t)rng_range(0, 256);
		h_eq_u64("lerp t=0", gfx_lerp(a, b, 0), a);
		h_eq_u64("lerp t=256", gfx_lerp(a, b, 256), b);
		h_eq_u64("lerp t>256", gfx_lerp(a, b, 4000000000u), b);
		h_eq_u64("lerp bleu", gfx_lerp(a, b, t) & 255,
			(((a & 255) * (256 - t) + (b & 255) * t + 128) >> 8) & 255);
		h_eq_u64("lerp alpha", gfx_lerp(a, b, t) >> 24,
			(((a >> 24) * (256 - t) + (b >> 24) * t + 128) >> 8) & 255);
		k++;
	}
	h_eq_u64("lerp milieu", gfx_lerp(0xff000000, 0xffffffff, 128), 0xff808080);
}

int	main(void)
{
	h_begin("a15/color");
	rng_seed(20261005);
	h_run("blend exhaustif (a, canal, d)", blend_exhaustif);
	h_run("blend bornes et alpha", blend_bornes);
	h_run("lerp bornes et formule", lerp_formule);
	h_eq_u64("argb", gfx_argb(0x12, 0x34, 0x56, 0x78), 0x12345678);
	h_eq_u64("argb zero", gfx_argb(0, 0, 0, 0), 0);
	h_eq_u64("argb max", gfx_argb(255, 255, 255, 255), 0xffffffffu);
	return (h_end());
}
