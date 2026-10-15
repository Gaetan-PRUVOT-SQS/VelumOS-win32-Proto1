#include <stdlib.h>
#include "harness.h"
#include "lt_gal.h"

#define DARKC 0xff202020u
#define LIGHTC 0xffe0e0e0u
#define GW 240
#define GH 64

static int32_t	grey(const t_lt *t, int32_t x, int32_t y)
{
	return ((int32_t)(t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]
		& 0xff));
}

static void	split_fill(t_lt *t)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < t->h)
	{
		x = 0;
		while (x < t->w)
		{
			t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x] = DARKC;
			if (x >= t->w / 2)
				t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x] = LIGHTC;
			x++;
		}
		y++;
	}
}

static void	blur_monotone(void)
{
	t_lt		t;
	uint32_t	*buf;
	size_t		n;
	int32_t		x;
	int32_t		bad;

	n = luna_glass_need(lp_rect(0, 0, GW, GH));
	buf = malloc(n * sizeof(uint32_t));
	h_true(buf != NULL && lt_open(&t, GW, GH) == 0, "allocation");
	luna_glass_scratch(buf, n);
	split_fill(&t);
	luna_glass(&t.s, lp_rect(0, 0, GW, GH), LUNA_GLASS_FLOAT);
	bad = 0;
	x = 41;
	while (++x < GW - 40)
		bad += grey(&t, x, GH / 2) < grey(&t, x - 1, GH / 2) - 1;
	h_eq_i64("transition croissante, sans oscillation", bad, 0);
	h_true(grey(&t, GW / 2 - 1, GH / 2) > grey(&t, 40, GH / 2) + 3,
		"le bord est adouci, pas coupe net");
	luna_glass_scratch(NULL, 0);
	free(buf);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_flou");
	h_run("flou monotone sur un bord net", blur_monotone);
	return (h_end());
}
