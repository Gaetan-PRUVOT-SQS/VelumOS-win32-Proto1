#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static int	make_win(t_handle c, t_rect r)
{
	return (hs_slot(&g_s, hs_create(&g_s, c, r, WS_DEFAULT)));
}

static void	extension_une_fenetre(void)
{
	t_rect	r;
	t_rect	got;
	t_rect	want;
	int		slot;

	hs_start(&g_s, 1024, 768);
	slot = make_win(hr_client(&g_s), rect_make(100, 100, 400, 300));
	r = g_s.t.w[slot].rect;
	got = wgl_extend(&g_s.t, rect_make(150, 120, 10, 10));
	want = rect_make(r.x, r.y, r.w, 50);
	h_true(!memcmp(&got, &want, sizeof(got)), "dans la bande : bande entiere");
	got = wgl_extend(&g_s.t, rect_make(700, 500, 20, 20));
	want = rect_make(700, 500, 20, 20);
	h_true(!memcmp(&got, &want, sizeof(got)), "hors verre : inchange");
	got = wgl_extend(&g_s.t, rect_make(150, 140, 10, 30));
	want = rect_make(r.x, r.y, r.w, 70);
	h_true(!memcmp(&got, &want, sizeof(got)), "a cheval : union avec la bande");
	got = wgl_extend(&g_s.t, rect_make(-50, 110, 100, 10));
	want = rect_make(0, 110, 50, 10);
	h_true(!memcmp(&got, &want, sizeof(got)), "borne par l'ecran");
	hs_stop(&g_s);
}

static void	extension_en_chaine(void)
{
	t_handle	c;
	t_rect		a;
	t_rect		b;
	t_rect		got;
	t_rect		want;

	hs_start(&g_s, 1024, 768);
	c = hr_client(&g_s);
	a = g_s.t.w[make_win(c, rect_make(100, 100, 400, 300))].rect;
	b = g_s.t.w[make_win(c, rect_make(450, 120, 300, 200))].rect;
	got = wgl_extend(&g_s.t, rect_make(110, 105, 5, 5));
	want = rect_make(a.x, a.y, b.x + b.w - a.x, b.y + 50 - a.y);
	h_true(!memcmp(&got, &want, sizeof(got)), "deux bandes chainees");
	hs_stop(&g_s);
}

static void	equivalence_deplacement(void)
{
	t_handle	c;
	int			top;
	t_surface	ref;
	uint32_t	*px;

	hs_start(&g_s, 1024, 768);
	c = hr_client(&g_s);
	make_win(c, rect_make(100, 100, 400, 300));
	top = make_win(c, rect_make(300, 120, 400, 300));
	ws_mark(&g_s, g_s.dirty.screen);
	ws_frame(&g_s);
	ws_apply_rect(&g_s, top, rect_make(520, 300, 400, 300), true);
	ws_frame(&g_s);
	px = calloc(1024 * 768, 4);
	gfx_surface_init(&ref, px, 1024, 768);
	h_eq_i64("incremental egal au complet", hx_compare(&g_s, &ref), 0);
	free(px);
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_glass_ext");
	h_run("extension, partition une fenetre", extension_une_fenetre);
	h_run("extension, fermeture transitive", extension_en_chaine);
	h_run("equivalence incremental complet apres deplacement",
		equivalence_deplacement);
	return (h_end());
}
