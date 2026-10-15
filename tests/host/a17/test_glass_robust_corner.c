#include <stdint.h>
#include <stdio.h>
#include "harness.h"
#include "luna_int.h"

#define NR 9
#define NP 9
#define RCAP 16777216

static const int32_t	g_r[NR] = {1, 2, 4096, 5792, 5793, 65536, 8388608,
	16777216, INT32_MAX};

static const int32_t	g_p[NP][2] = {{0, 0}, {1, 1}, {2, 2}, {0, 1}, {1, 0},
{3, 3}, {3, 2}, {4, 4}, {4, 0}};

static int64_t	pick(int32_t r, int32_t k)
{
	if (k == 0)
		return (0);
	if (k == 1)
		return ((int64_t)r - 1);
	if (k == 2)
		return (r / 2);
	if (k == 3)
		return (r / 4);
	return (-1);
}

static int32_t	model_cov(int64_t r, int64_t ix, int64_t iy)
{
	int64_t	s;
	int64_t	lim;
	int64_t	dx;
	int64_t	dy;
	int32_t	n;

	n = 0;
	s = 0;
	lim = 2 * r + 1;
	while (s < 16 && ix >= 0 && iy >= 0 && ix <= lim && iy <= lim)
	{
		dx = r * 8 - (ix * 8 + (s % 4) * 2 + 1);
		dy = r * 8 - (iy * 8 + (s / 4) * 2 + 1);
		n += (dx * dx + dy * dy <= r * r * 64);
		s++;
	}
	return (n);
}

static void	one_case(int32_t i, int32_t k)
{
	char		name[64];
	int64_t		ix;
	int64_t		iy;
	int32_t		r;
	int32_t		want;

	r = g_r[i];
	ix = pick(r, g_p[k][0]);
	iy = pick(r, g_p[k][1]);
	want = model_cov(lp_min(r, RCAP), ix, iy);
	snprintf(name, sizeof(name), "rayon %d, pixel %d", r, k);
	h_eq_i64(name, lp_corner_cov(r, (int32_t)ix, (int32_t)iy), want);
}

static void	cov_table(void)
{
	int32_t	i;
	int32_t	k;

	i = -1;
	while (++i < NR)
	{
		k = -1;
		while (++k < NP)
			one_case(i, k);
	}
}

int	main(void)
{
	h_begin("a17/glass_robuste_coin");
	h_run("couverture de coin, rayons limites, modele 64 bits", cov_table);
	return (h_end());
}
