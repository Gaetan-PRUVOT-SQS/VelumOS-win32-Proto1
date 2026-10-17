#include "harness.h"
#include "velum/luna_launcher.h"
#include "lt.h"
#include "launcher_int.h"

static t_lt	g_t;

static uint64_t	drawn_hash(int32_t w, int32_t h)
{
	t_launcher	l;

	l.user = "Nom";
	l.tiles = NULL;
	l.ntiles = 0;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	lt_clear(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, lp_rect(12, 20, w, h));
	return (lt_hash(&g_t.s));
}

static void	min_size(void)
{
	uint64_t	blank;

	lt_clear(&g_t, LC_BLACK);
	blank = lt_hash(&g_t.s);
	h_true(drawn_hash(LL_MIN_W - 1, LUNA_LAUNCHER_HEIGHT) == blank,
		"largeur minimale - 1 : rien");
	h_true(drawn_hash(LL_MIN_W, LUNA_LAUNCHER_HEIGHT) != blank,
		"largeur minimale : dessine");
	h_true(drawn_hash(540, LUNA_LAUNCHER_HEIGHT - 1) == blank,
		"hauteur 288 : les rangees se chevaucheraient, rien");
	h_true(drawn_hash(540, LUNA_LAUNCHER_HEIGHT) != blank,
		"hauteur 289 : dessine");
}

static void	order(const t_launcherlayout *l)
{
	int32_t	i;

	i = 0;
	while (i + 1 < l->ntiles)
	{
		h_true(l->tile[i].x + l->tile[i].w < l->tile[i + 1].x,
			"tuiles disjointes");
		i++;
	}
	h_true(l->tile[l->ntiles - 1].x + l->tile[l->ntiles - 1].w
		<= l->search.x + l->search.w, "derniere tuile dans la marge");
	h_true(l->avatar.x + l->avatar.w <= l->user.x, "avatar avant le nom");
	h_true(l->user.x + l->user.w <= l->logoff.x, "nom avant le bouton");
	h_true(l->logoff.x + l->logoff.w <= l->poweroff.x, "boutons disjoints");
	h_true(l->poweroff.x + l->poweroff.w <= l->panel.x + l->panel.w - LL_PAD,
		"bouton rond dans la marge");
}

static void	layout_widths(void)
{
	static const int32_t	w[6] = {LL_MIN_W, LL_MIN_W + 1, 400, 539, 540, 1000};
	t_launcherlayout		l;
	int32_t					i;

	i = 0;
	while (i < 6)
	{
		luna_launcher_layout(lp_rect(12, 431, w[i], 289), 4, &l);
		order(&l);
		h_true(l.tile[0].w > 0, "tuile non vide");
		i++;
	}
}

int	main(void)
{
	if (lt_open(&g_t, 640, 480) != 0)
		return (1);
	h_begin("a17/lanceur_minimum");
	h_run("taille minimale : valeurs limites", min_size);
	h_run("largeurs : aucun recouvrement", layout_widths);
	lt_close(&g_t);
	return (h_end());
}
