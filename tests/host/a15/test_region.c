#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	expect_rect(const char *what, const t_region *rg, t_rect want)
{
	h_eq_i64(what, rg->n, 1);
	h_eq_i64(what, rg->r[0].x, want.x);
	h_eq_i64(what, rg->r[0].y, want.y);
	h_eq_i64(what, rg->r[0].w, want.w);
	h_eq_i64(what, rg->r[0].h, want.h);
}

static void	merge_cases(void)
{
	t_region	rg;

	region_clear(&rg);
	region_add(&rg, rect_make(0, 0, 10, 10));
	region_add(&rg, rect_make(10, 0, 10, 10));
	expect_rect("voisins horizontaux fusionnes", &rg, rect_make(0, 0, 20, 10));
	region_add(&rg, rect_make(0, 10, 20, 5));
	expect_rect("voisin vertical fusionne", &rg, rect_make(0, 0, 20, 15));
	region_clear(&rg);
	region_add(&rg, rect_make(0, 0, 10, 10));
	region_add(&rg, rect_make(10, 1, 10, 10));
	h_eq_i64("voisins decales: non fusionnes", rg.n, 2);
	region_add(&rg, rect_make(2, 2, 3, 3));
	h_eq_i64("rect contenu: sans effet", rg.n, 2);
	region_add(&rg, rect_make(5, 5, 10, 10));
	h_true(rg_valid(&rg), "invariants apres chevauchement");
}

static void	subtract_cases(void)
{
	t_region	rg;

	region_clear(&rg);
	region_add(&rg, rect_make(0, 0, 10, 10));
	region_subtract(&rg, rect_make(3, 3, 4, 4));
	h_eq_i64("trou: 4 morceaux", rg.n, 4);
	region_subtract(&rg, rect_make(-5, -5, 30, 30));
	h_true(region_empty(&rg), "tout retire");
	region_add(&rg, rect_make(0, 0, 10, 10));
	region_subtract(&rg, rect_make(20, 20, 5, 5));
	expect_rect("sans recouvrement: inchange", &rg, rect_make(0, 0, 10, 10));
	region_subtract(&rg, rect_make(5, 0, 5, 10));
	expect_rect("moitie retiree", &rg, rect_make(0, 0, 5, 10));
	region_subtract(&rg, rect_make(0, 0, 0, 5));
	expect_rect("rect vide: inchange", &rg, rect_make(0, 0, 5, 10));
}

static void	intersect_bounds(void)
{
	t_region	rg;
	t_rect		b;

	region_clear(&rg);
	h_true(region_empty(&rg), "vide au depart");
	b = region_bounds(&rg);
	h_true(rect_empty(b), "enveloppe vide");
	region_add(&rg, rect_make(0, 0, 4, 4));
	region_add(&rg, rect_make(10, 10, 5, 5));
	b = region_bounds(&rg);
	h_eq_i64("enveloppe x", b.x, 0);
	h_eq_i64("enveloppe w", b.w, 15);
	h_eq_i64("enveloppe h", b.h, 15);
	region_intersect(&rg, rect_make(2, 2, 10, 10));
	h_eq_i64("intersection: deux morceaux", rg.n, 2);
	region_intersect(&rg, rect_make(100, 100, 1, 1));
	h_true(region_empty(&rg), "intersection vide");
}

int	main(void)
{
	h_begin("a15/region");
	h_run("region: fusion des voisins", merge_cases);
	h_run("region: soustraction", subtract_cases);
	h_run("region: intersection et enveloppe", intersect_bounds);
	return (h_end());
}
