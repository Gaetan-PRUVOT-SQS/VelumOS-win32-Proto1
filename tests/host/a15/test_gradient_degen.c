#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	degenerate_rows(void)
{
	t_scene		sc;
	t_gradient	g;

	scene_open(&sc, 7, 5, 0);
	g = grad_make(rect_make(0, 0, 7, 1), 0xff102030, 0xffa0b0c0, false);
	gfx_gradient(&sc.s, &g);
	h_eq_u64("une ligne: from", scene_px(&sc, 3, 0), 0xff102030);
	g = grad_make(rect_make(0, 1, 7, 2), 0xff102030, 0xffa0b0c0, false);
	gfx_gradient(&sc.s, &g);
	h_eq_u64("deux lignes: from", scene_px(&sc, 3, 1), 0xff102030);
	h_eq_u64("deux lignes: to", scene_px(&sc, 3, 2), 0xffa0b0c0);
	scene_adopt(&sc, rect_make(0, 0, 7, 5));
	scene_close(&sc, "cas degeneres: lignes");
}

static void	degenerate_column(void)
{
	t_scene		sc;
	t_gradient	g;

	scene_open(&sc, 7, 5, 0);
	g = grad_make(rect_make(2, 0, 1, 5), 0xff010203, 0xff0a0b0c, true);
	gfx_gradient(&sc.s, &g);
	h_eq_u64("une colonne horizontale: from", scene_px(&sc, 2, 4), 0xff010203);
	g = grad_make(rect_make(0, 0, 7, 5), 0xff303030, 0xff303030, true);
	gfx_gradient(&sc.s, &g);
	h_eq_u64("couleurs egales: uniforme", scene_px(&sc, 5, 3), 0xff303030);
	scene_adopt(&sc, rect_make(0, 0, 7, 5));
	scene_close(&sc, "cas degeneres: colonne");
}

int	main(void)
{
	h_begin("a15/gradient_degen");
	h_run("gradient: lignes degenerees", degenerate_rows);
	h_run("gradient: colonne et couleurs egales", degenerate_column);
	return (h_end());
}
