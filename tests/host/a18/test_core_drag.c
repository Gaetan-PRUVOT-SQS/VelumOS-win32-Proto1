#include "harness.h"
#include "help.h"
#include "velum/theme.h"

static t_wtable	g_t;

static t_rect	drag(uint32_t ht, int32_t dx, int32_t dy)
{
	t_wdrag	d;
	int		slot;

	hz_screen(&g_t);
	slot = hz_add(&g_t, WS_DEFAULT);
	g_t.w[slot].rect = rect_make(200, 200, 400, 300);
	wd_begin(&d, 0, ht, (t_point){300, 300});
	d.orig = g_t.w[0].rect;
	return (wd_target(&d, &g_t, (t_point){300 + dx, 300 + dy}));
}

static void	handles(void)
{
	static const int32_t	cases[8][5] = {{HT_LEFT, 185, 200, 415, 300},
	{HT_RIGHT, 200, 200, 415, 300}, {HT_TOP, 200, 175, 400, 325},
	{HT_BOTTOM, 200, 200, 400, 325}, {HT_TOPLEFT, 185, 175, 415, 325},
	{HT_TOPRIGHT, 200, 175, 415, 325}, {HT_BOTTOMLEFT, 185, 200, 415, 325},
	{HT_BOTTOMRIGHT, 200, 200, 415, 325}};
	t_rect					r;
	int						i;

	i = 0;
	while (i < 8)
	{
		r = drag((uint32_t)cases[i][0], 15 - 30 * (cases[i][1] == 185), 25
				- 50 * (cases[i][2] == 175));
		h_true(r.x == cases[i][1] && r.y == cases[i][2]
			&& r.w == cases[i][3] && r.h == cases[i][4], "poignee");
		h_true(wd_is_resize((uint32_t)cases[i][0]), "poignee reconnue");
		i++;
	}
	h_true(!wd_is_resize(HT_CAPTION) && !wd_is_resize(HT_CLIENT)
		&& !wd_is_resize(HT_NOWHERE) && !wd_is_resize(HT_CLOSE),
		"autres zones");
}

static void	limits(void)
{
	t_rect	r;

	r = drag(HT_RIGHT, -1000, 0);
	h_true(r.x == 200 && r.w == WS_MINTRACK_W, "largeur mini, gauche fixe");
	r = drag(HT_LEFT, 1000, 0);
	h_true(r.x + r.w == 600 && r.w == WS_MINTRACK_W, "mini, droite fixe");
	r = drag(HT_BOTTOM, 0, -1000);
	h_true(r.y == 200 && r.h == VT_CAPTION_H, "hauteur mini = barre de titre");
	r = drag(HT_TOP, 0, -1000);
	h_true(r.y == 0 && r.y + r.h == 500, "haut borne a la zone de travail");
	r = drag(HT_BOTTOMRIGHT, 5000, 5000);
	h_true(r.w <= 1024 && r.h <= 768, "plafond taille ecran");
	r = drag(HT_TOPLEFT, -5000, -5000);
	h_true(r.w <= 1024 && r.y >= 0, "coin haut gauche borne");
	r = drag(HT_CAPTION, 10, 20);
	h_true(r.x == 210 && r.y == 220 && r.w == 400, "deplacement simple");
	r = drag(HT_CAPTION, -5000, -5000);
	h_true(r.x == -(400 - WS_DRAG_KEEP) && r.y == 0, "deplacement borne");
	r = drag(HT_CAPTION, 5000, 5000);
	h_true(r.x == 1024 - WS_DRAG_KEEP
		&& r.y == g_t.work.y + g_t.work.h - VT_CAPTION_H, "legende accessible");
}

static void	cursors(void)
{
	h_eq_i64("gauche", wd_cursor(HT_LEFT), CUR_SIZE_WE);
	h_eq_i64("droite", wd_cursor(HT_RIGHT), CUR_SIZE_WE);
	h_eq_i64("haut", wd_cursor(HT_TOP), CUR_SIZE_NS);
	h_eq_i64("bas", wd_cursor(HT_BOTTOM), CUR_SIZE_NS);
	h_eq_i64("haut gauche", wd_cursor(HT_TOPLEFT), CUR_SIZE_NWSE);
	h_eq_i64("bas droite", wd_cursor(HT_BOTTOMRIGHT), CUR_SIZE_NWSE);
	h_eq_i64("haut droite", wd_cursor(HT_TOPRIGHT), CUR_SIZE_NESW);
	h_eq_i64("bas gauche", wd_cursor(HT_BOTTOMLEFT), CUR_SIZE_NESW);
	h_eq_i64("legende", wd_cursor(HT_CAPTION), CUR_ARROW);
}

int	main(void)
{
	h_begin("a18/core_drag");
	h_run("huit poignees", handles);
	h_run("limites", limits);
	h_run("curseurs de bord", cursors);
	return (h_end());
}
