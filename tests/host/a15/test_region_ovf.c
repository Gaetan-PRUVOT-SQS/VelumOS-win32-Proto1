#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	add_overflow(void)
{
	t_region	rg;
	int32_t		i;
	int32_t		j;

	region_clear(&rg);
	i = 0;
	while (i < 45)
	{
		region_add(&rg, rect_make(3 * i, 0, 1, 1));
		h_true(rg.n <= GFX_REGION_MAX, "n borne");
		j = 0;
		while (j <= i)
		{
			h_true(rg_covers(&rg, 3 * j, 0), "aucune zone sale perdue");
			j++;
		}
		if (i == GFX_REGION_MAX)
		{
			h_eq_i64("33e ajout: remplace par l'enveloppe", rg.n, 1);
			h_eq_i64("enveloppe x", rg.r[0].x, 0);
			h_eq_i64("enveloppe w", rg.r[0].w, 3 * GFX_REGION_MAX + 1);
		}
		i++;
	}
}

static void	subtract_overflow(void)
{
	t_region	rg;
	int32_t		i;

	region_clear(&rg);
	i = 0;
	while (i < 17)
		region_add(&rg, rect_make(4 * i++, 0, 2, 10));
	h_eq_i64("17 rectangles disjoints", rg.n, 17);
	region_subtract(&rg, rect_make(0, 4, 100, 2));
	h_true(rg.n <= GFX_REGION_MAX, "n borne apres soustraction");
	i = 0;
	while (i < 17)
	{
		h_true(rg_covers(&rg, 4 * i, 0), "zone restante haute conservee");
		h_true(rg_covers(&rg, 4 * i + 1, 9), "zone restante basse conservee");
		i++;
	}
}

static void	extremes_and_null(void)
{
	t_region	rg;

	region_clear(NULL);
	region_add(NULL, rect_make(0, 0, 1, 1));
	region_subtract(NULL, rect_make(0, 0, 1, 1));
	region_intersect(NULL, rect_make(0, 0, 1, 1));
	h_true(region_empty(NULL), "region nulle vide");
	h_true(rect_empty(region_bounds(NULL)), "enveloppe nulle vide");
	region_clear(&rg);
	region_add(&rg, rect_make(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX));
	h_eq_i64("rect extreme borne", rg.n, 1);
	region_add(&rg, rect_make(INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX));
	region_add(&rg, rect_make(5, 5, 0, 5));
	region_add(&rg, rect_make(5, 5, -5, 5));
	h_true(region_bounds(&rg).w > 0, "enveloppe extreme representable");
	region_subtract(&rg, rect_make(INT32_MIN, INT32_MIN, INT32_MAX,
			INT32_MAX));
	h_true(rg.n <= GFX_REGION_MAX, "n borne");
}

static void	corrupted_count(void)
{
	t_region	rg;

	region_clear(&rg);
	rg.n = 1000;
	region_add(&rg, rect_make(0, 0, 3, 3));
	h_true(rg.n <= GFX_REGION_MAX, "n corrompu borne (add)");
	rg.n = 4000000000u;
	region_subtract(&rg, rect_make(0, 0, 3, 3));
	h_true(rg.n <= GFX_REGION_MAX, "n corrompu borne (subtract)");
	rg.n = 77;
	region_intersect(&rg, rect_make(0, 0, 3, 3));
	h_true(rg.n <= GFX_REGION_MAX, "n corrompu borne (intersect)");
	rg.n = 99;
	h_true(rg.n > GFX_REGION_MAX, "etat prepare");
	(void)region_bounds(&rg);
	h_true(!region_empty(&rg), "region non vide");
}

int	main(void)
{
	h_begin("a15/region_ovf");
	h_run("region: debordement a l'ajout", add_overflow);
	h_run("region: debordement a la soustraction", subtract_overflow);
	h_run("region: extremes et pointeur nul", extremes_and_null);
	h_run("region: compteur corrompu", corrupted_count);
	return (h_end());
}
