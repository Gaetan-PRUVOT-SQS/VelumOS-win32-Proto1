#include "harness.h"
#include "help.h"

static t_wtable	g_t;

static void	ids_generation(void)
{
	int			a;
	uint32_t	old;

	wt_init(&g_t, rect_make(0, 0, 640, 480));
	a = hz_add(&g_t, WS_DEFAULT);
	h_eq_i64("premier slot", a, 0);
	old = g_t.w[a].id;
	h_true((old & 0xff) == 1 && old != 0, "id = generation | slot+1");
	h_eq_i64("retrouve", wt_find(&g_t, old), a);
	wt_free(&g_t, a);
	h_eq_i64("libere introuvable", wt_find(&g_t, old), -1);
	a = hz_add(&g_t, WS_DEFAULT);
	h_eq_i64("slot reutilise", a, 0);
	h_true(g_t.w[a].id != old, "generation differente");
	h_eq_i64("ancien id refuse", wt_find(&g_t, old), -1);
	h_eq_i64("id nul", wt_find(&g_t, 0), -1);
	h_eq_i64("slot 0", wt_find(&g_t, 0x100), -1);
	h_eq_i64("slot 129", wt_find(&g_t, 0x181), -1);
	h_eq_i64("slot 255", wt_find(&g_t, 0x1ff), -1);
	h_eq_i64("invariants", wz_check(&g_t), 0);
}

static void	quota_table(void)
{
	int	i;

	wt_init(&g_t, rect_make(0, 0, 640, 480));
	i = 0;
	while (i < WS_WIN_MAX)
	{
		h_true(hz_add(&g_t, WS_DEFAULT) == i, "remplissage");
		i++;
	}
	h_eq_i64("table pleine", wt_alloc(&g_t, 0), E_NOSPC);
	h_eq_i64("compte proprietaire", wt_count_owner(&g_t, 0), WS_WIN_MAX);
	h_eq_i64("autre proprietaire", wt_count_owner(&g_t, 1), 0);
	h_eq_i64("invariants plein", wz_check(&g_t), 0);
	wt_free(&g_t, 5);
	wt_free(&g_t, 5);
	wt_free(&g_t, -1);
	wt_free(&g_t, WS_WIN_MAX);
	h_eq_i64("double liberation sans effet", g_t.nused, WS_WIN_MAX - 1);
	h_eq_i64("invariants apres", wz_check(&g_t), 0);
}

static void	bands(void)
{
	int	top;
	int	desk;
	int	n1;
	int	n2;

	wt_init(&g_t, rect_make(0, 0, 640, 480));
	top = hz_add(&g_t, WS_TOPMOST);
	n1 = hz_add(&g_t, WS_DEFAULT);
	desk = hz_add(&g_t, WS_DESKTOP);
	n2 = hz_add(&g_t, WS_APPBAR);
	h_eq_i64("bureau au fond", g_t.z[0], desk);
	h_eq_i64("normale ensuite", g_t.z[1], n1);
	h_eq_i64("barre au-dessus", wz_band(g_t.w[g_t.z[3]].style), WS_BAND_TOP);
	h_true(!wz_raise(&g_t, n1), "normale deja en haut de sa bande");
	n2 = hz_add(&g_t, WS_DEFAULT);
	h_eq_i64("nouvelle normale sous les topmost", g_t.z[2], n2);
	h_true(wz_raise(&g_t, n1), "remontee change l'ordre");
	h_eq_i64("remontee sous les topmost", g_t.z[2], n1);
	h_true(wz_raise(&g_t, top) || g_t.z[3] == top, "topmost en haut");
	h_true(!wz_raise(&g_t, desk), "bureau reste au fond");
	h_eq_i64("index absent", wz_index(&g_t, 77), -1);
	h_eq_i64("invariants", wz_check(&g_t), 0);
}

static void	check_detects(void)
{
	int	a;
	int	b;

	wt_init(&g_t, rect_make(0, 0, 640, 480));
	a = hz_add(&g_t, WS_DEFAULT);
	b = hz_add(&g_t, WS_TOPMOST);
	g_t.z[0] = (uint16_t)b;
	g_t.z[1] = (uint16_t)a;
	h_true(wz_check(&g_t) < 0, "bande inversee detectee");
	g_t.z[0] = (uint16_t)a;
	g_t.z[1] = (uint16_t)a;
	h_true(wz_check(&g_t) < 0, "doublon detecte");
	g_t.z[1] = (uint16_t)b;
	g_t.active = a;
	g_t.w[a].state = WSTATE_MIN;
	h_true(wz_check(&g_t) < 0, "active reduite detectee");
	g_t.w[a].state = WSTATE_NORMAL;
	g_t.w[a].style |= WS_NOACTIVATE;
	h_true(wz_check(&g_t) < 0, "active non activable detectee");
	g_t.w[a].style = WS_DEFAULT;
	g_t.nz = 1;
	h_true(wz_check(&g_t) < 0, "fenetre hors ordre Z detectee");
}

int	main(void)
{
	h_begin("a18/core_z");
	h_run("identifiants et generations", ids_generation);
	h_run("quota de la table", quota_table);
	h_run("bandes de l'ordre Z", bands);
	h_run("detection des violations", check_detects);
	return (h_end());
}
