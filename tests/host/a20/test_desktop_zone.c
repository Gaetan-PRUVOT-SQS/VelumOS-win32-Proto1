#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "layout.h"
#include "desktop.h"

static void	desktop_decalage_de_zone(void)
{
	t_rect	out[DESK_ICONS];

	h_eq_i64("zone decalee", desk_grid(lay_rect(10, 30, 1024, 700), 1, out),
		1);
	h_eq_i64("origine decalee: x", out[0].x, 10 + 24);
	h_eq_i64("origine decalee: y", out[0].y, 30 + 24);
}

static void	desktop_rangees_aux_limites(void)
{
	h_eq_i64("pas de rangee sous 105", desk_rows(lay_rect(0, 0, 400, 104)),
		0);
	h_eq_i64("une rangee a 105", desk_rows(lay_rect(0, 0, 400, 105)), 1);
	h_eq_i64("une rangee a 203", desk_rows(lay_rect(0, 0, 400, 203)), 1);
	h_eq_i64("deux rangees a 204", desk_rows(lay_rect(0, 0, 400, 204)), 2);
	h_eq_i64("sept rangees a 699", desk_rows(lay_rect(0, 0, 400, 699)), 7);
	h_eq_i64("six rangees a 698", desk_rows(lay_rect(0, 0, 400, 698)), 6);
}

static void	desktop_libelles(void)
{
	h_true(desk_icon(0) != NULL && desk_icon(2) != NULL, "trois icones");
	h_true(desk_icon(DESK_ICONS) == NULL, "indice hors liste");
	h_eq_i64("ordre: Ordinateur", desk_icon(0)->action, DA_COMPUTER);
	h_eq_i64("ordre: Corbeille", desk_icon(1)->action, DA_RECYCLE);
	h_eq_i64("ordre: Bonjour", desk_icon(2)->action, DA_HELLO);
}

int	main(void)
{
	h_begin("a20/desktop-zone");
	h_run("bureau: decalage de zone", desktop_decalage_de_zone);
	h_run("bureau: rangees aux limites", desktop_rangees_aux_limites);
	h_run("bureau: ordre des icones", desktop_libelles);
	return (h_end());
}
