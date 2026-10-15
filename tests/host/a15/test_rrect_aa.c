#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	check_stat(t_cstat st, int32_t rad)
{
	double	want;

	h_true(st.max_err <= 33, "couverture proche du calcul fin (max)");
	h_true(st.abs_err <= (int64_t)rad * rad * 3 / 2 + 8, "erreur moyenne");
	want = 3.14159265358979 * rad * rad / 4.0;
	h_true(fabs((double)st.area / 255.0 - want) <= want * 0.02 + 1.5,
		"aire du quart de disque");
}

static void	aa_one(t_rrect rr)
{
	t_scene	sc;
	t_rect	r;

	r = rr.r;
	scene_open(&sc, 64, 64, 0);
	scene_clear(&sc, 0xff000000);
	gfx_rrect_fill(&sc.s, &rr);
	check_stat(corner_stat(&sc, r, 0, rr.tl), rr.tl);
	check_stat(corner_stat(&sc, r, 1, rr.tr), rr.tr);
	check_stat(corner_stat(&sc, r, 3, rr.br), rr.br);
	check_stat(corner_stat(&sc, r, 2, rr.bl), rr.bl);
	scene_rect(&sc, r, 0xffffffff);
	scene_adopt(&sc, rect_make(r.x, r.y, rr.tl, rr.tl));
	scene_adopt(&sc, rect_make(r.x + r.w - rr.tr, r.y, rr.tr, rr.tr));
	scene_adopt(&sc, rect_make(r.x + r.w - rr.br, r.y + r.h - rr.br, rr.br,
			rr.br));
	scene_adopt(&sc, rect_make(r.x, r.y + r.h - rr.bl, rr.bl, rr.bl));
	scene_close(&sc, "rrect lisse: hors coins plein ou inchange");
}

static void	aa_vs_fine(void)
{
	static const int32_t	radii[6] = {4, 5, 8, 12, 20, 27};
	t_rrect					rr;
	int						i;

	i = 0;
	while (i < 6)
	{
		rr = (t_rrect){rect_make(4, 4, 56, 56), 0xffffffff, radii[i],
			radii[(i + 1) % 6], radii[(i + 2) % 6], radii[(i + 3) % 6]};
		aa_one(rr);
		i++;
	}
}

int	main(void)
{
	h_begin("a15/rrect_aa");
	rng_seed(20261005);
	h_run("rrect lisse: couverture contre echantillonnage fin", aa_vs_fine);
	return (h_end());
}
