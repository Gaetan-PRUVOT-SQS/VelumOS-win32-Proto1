#include <string.h>
#include "harness.h"
#include "ref.h"

bool	rg_covers(const t_region *rg, int64_t x, int64_t y)
{
	uint32_t	i;

	i = 0;
	while (i < rg->n)
	{
		if (in_rect(rg->r[i], x, y))
			return (true);
		i++;
	}
	return (false);
}

bool	rg_superset(const uint8_t got[RGN_G * RGN_G],
		const uint8_t want[RGN_G * RGN_G])
{
	int	i;

	i = 0;
	while (i < RGN_G * RGN_G)
	{
		if (want[i] && !got[i])
			return (false);
		i++;
	}
	return (true);
}

t_rect	rg_cells_bounds(const uint8_t cells[RGN_G * RGN_G])
{
	int32_t	x;
	int32_t	y;
	t_rect	b;

	b = rect_make(0, 0, 0, 0);
	y = 0;
	while (y < RGN_G)
	{
		x = 0;
		while (x < RGN_G)
		{
			if (cells[y * RGN_G + x])
				b = rect_union(b, rect_make(x, y, 1, 1));
			x++;
		}
		y++;
	}
	return (b);
}

static void	step_checks(const t_region *rg, const uint8_t want[RGN_G * RGN_G],
		uint32_t n0, t_rgstat *st)
{
	uint8_t	got[RGN_G * RGN_G];
	t_rect	b;

	h_eq_i64("rectangles disjoints", rg_paint(got, rg), 0);
	h_true(rg_valid(rg), "invariants: non vides, bornes, voisins fusionnes");
	h_true(rg_superset(got, want), "aucune zone sale perdue");
	b = rg_cells_bounds(got);
	h_eq_i64("region_bounds x", region_bounds(rg).x, b.x);
	h_eq_i64("region_bounds w", region_bounds(rg).w, b.w);
	h_eq_i64("region_bounds h", region_bounds(rg).h, b.h);
	h_true(region_empty(rg) == (rg->n == 0), "region_empty coherent");
	st->steps++;
	if (memcmp(got, want, sizeof(got)) == 0)
		st->exact++;
	else
	{
		st->fallback++;
		h_true(n0 > 6, "pas de repli en dessous de 7 rectangles");
		h_eq_i64("repli par un seul rectangle", rg->n, 1);
	}
	if (rg->n > st->max_n)
		st->max_n = rg->n;
}

void	rg_step(t_region *rg, int op, t_rect r, t_rgstat *st)
{
	uint8_t		want[RGN_G * RGN_G];
	uint32_t	n0;

	n0 = rg->n;
	rg_paint(want, rg);
	rg_model(want, op, r);
	rg_apply(rg, op, r);
	step_checks(rg, want, n0, st);
}
