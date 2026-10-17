#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	copie_du_rectangle_etendu(void)
{
	t_rect	win;
	int		slot;

	hs_start(&g_s, 1024, 768);
	slot = hs_slot(&g_s, hs_create(&g_s, hr_client(&g_s),
				rect_make(100, 100, 400, 300), WS_DEFAULT));
	win = g_s.t.w[slot].rect;
	h_eq_i64("dans la bande : bande entiere copiee, rien d'autre",
		hc_frame(&g_s, rect_make(150, 120, 10, 10),
			rect_make(win.x, win.y, win.w, 50)), 0);
	h_eq_i64("hors verre : le rectangle sale seul",
		hc_frame(&g_s, rect_make(700, 500, 20, 20),
			rect_make(700, 500, 20, 20)), 0);
	hs_stop(&g_s);
}

static void	rectangles_sales_degeneres(void)
{
	hs_start(&g_s, 1024, 768);
	ws_frame(&g_s);
	ws_mark(&g_s, rect_make(-5000, -5000, 10, 10));
	ws_mark(&g_s, rect_make(0, 0, 0, 0));
	ws_mark(&g_s, rect_make(100, 100, -4, -4));
	ws_mark(&g_s, rect_make(1024, 768, 10, 10));
	ws_mark(&g_s, rect_make(INT32_MAX - 3, INT32_MAX - 3, 10, 10));
	ws_mark(&g_s, rect_make(INT32_MIN, INT32_MIN, 10, 10));
	ws_mark(&g_s, rect_make(INT32_MIN, 0, INT32_MAX, 10));
	h_eq_u64("aucun rectangle retenu", g_s.dirty.n, 0);
	ws_frame(&g_s);
	h_eq_i64("rectangle partiellement hors ecran, coin bas droit",
		hc_frame(&g_s, rect_make(1020, 764, 100, 100),
			rect_make(1020, 764, 4, 4)), 0);
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_glass_copy");
	h_run("copie : le rectangle etendu et rien d'autre",
		copie_du_rectangle_etendu);
	h_run("rectangles sales degeneres, valeurs limites",
		rectangles_sales_degeneres);
	return (h_end());
}
