#include "harness.h"
#include "fake.h"

static t_droid	g_d;

static void	lay_vide_et_un_texte(void)
{
	t_apklay	l;

	fd_reset(&g_d);
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("arbre vide", l.n, 0);
	apklay_run(NULL, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("modele nul", l.n, 0);
	g_d.root = fd_add(&g_d, DV_TEXT, 0);
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("un texte", l.n, 1);
	h_eq_i64("x", l.box[0].r.x, 12);
	h_eq_i64("y", l.box[0].r.y, 12);
	h_eq_i64("w", l.box[0].r.w, 376);
	h_eq_i64("h", l.box[0].r.h, 20);
	h_eq_u64("genre", l.box[0].kind, DV_TEXT);
	h_eq_u64("vue", l.box[0].view, 1);
}

static void	lay_vertical_texte_bouton(void)
{
	t_apklay	l;

	fd_reset(&g_d);
	g_d.root = fd_add(&g_d, DV_LINEAR, 0);
	fd_add(&g_d, DV_TEXT, 1);
	fd_add(&g_d, DV_BUTTON, 1);
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("deux boites", l.n, 2);
	h_eq_i64("texte y", l.box[0].r.y, 12);
	h_eq_i64("texte h", l.box[0].r.h, 20);
	h_eq_i64("bouton y", l.box[1].r.y, 44);
	h_eq_i64("bouton h", l.box[1].r.h, 26);
	h_eq_i64("bouton w", l.box[1].r.w, 376);
	h_eq_u64("bouton genre", l.box[1].kind, DV_BUTTON);
	h_true(fd_inside(&l, (t_rect){12, 12, 376, 276}), "dans la zone");
	h_true(fd_disjoint(&l), "sans recouvrement");
}

static void	lay_horizontal_trois_boutons(void)
{
	t_apklay	l;

	fd_reset(&g_d);
	g_d.root = fd_add(&g_d, DV_LINEAR, 0);
	g_d.views[0].orientation = DROID_HORIZONTAL;
	fd_add(&g_d, DV_BUTTON, 1);
	fd_add(&g_d, DV_BUTTON, 1);
	fd_add(&g_d, DV_BUTTON, 1);
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("trois boites", l.n, 3);
	h_eq_i64("largeur", l.box[0].r.w, 117);
	h_eq_i64("x0", l.box[0].r.x, 12);
	h_eq_i64("x1", l.box[1].r.x, 141);
	h_eq_i64("x2", l.box[2].r.x, 270);
	h_eq_i64("y2", l.box[2].r.y, 12);
	h_eq_i64("h2", l.box[2].r.h, 26);
	h_true(fd_inside(&l, (t_rect){12, 12, 376, 276}), "dans la zone");
	h_true(fd_disjoint(&l), "sans recouvrement");
}

static void	lay_seize_enfants_et_zone_petite(void)
{
	t_apklay	l;
	int			i;

	fd_reset(&g_d);
	g_d.root = fd_add(&g_d, DV_LINEAR, 0);
	i = 0;
	while (i++ < 16)
		fd_add(&g_d, DV_BUTTON, 1);
	apklay_run(&g_d, (t_rect){0, 0, 400, 2000}, &l);
	h_eq_u64("seize boites", l.n, 16);
	h_eq_i64("derniere y", l.box[15].r.y, 12 + 15 * 38);
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("sept tiennent en 276", l.n, 7);
	h_true(fd_inside(&l, (t_rect){12, 12, 376, 276}), "dans la zone");
	apklay_run(&g_d, (t_rect){0, 0, 400, 49}, &l);
	h_eq_u64("zone de 25 de haut", l.n, 0);
	apklay_run(&g_d, (t_rect){0, 0, 400, 50}, &l);
	h_eq_u64("zone de 26 de haut", l.n, 1);
	apklay_run(&g_d, (t_rect){0, 0, 24, 24}, &l);
	h_eq_u64("zone sans interieur", l.n, 0);
	apklay_run(&g_d, (t_rect){0, 0, 31, 300}, &l);
	h_eq_u64("zone trop etroite", l.n, 0);
}

int	main(void)
{
	h_begin("d12 mise en page");
	h_run("lay_vide_et_un_texte", lay_vide_et_un_texte);
	h_run("lay_vertical_texte_bouton", lay_vertical_texte_bouton);
	h_run("lay_horizontal_trois_boutons", lay_horizontal_trois_boutons);
	h_run("lay_seize_enfants_et_zone_petite",
		lay_seize_enfants_et_zone_petite);
	return (h_end());
}
