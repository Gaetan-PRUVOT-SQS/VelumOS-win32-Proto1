#include "harness.h"
#include "velum/luna_launcher.h"
#include "lt.h"
#include "launcher_int.h"

static t_lt			g_t;
static t_lmodel		g_m;
static const t_rect	g_panel = {12, 20, 540, 289};

static void	garnir(void)
{
	static const t_lmapp	apps[] = {
		{"Bonjour", 0, ICON_PROGRAM},
		{"Calculatrice", 1, ICON_PROGRAM},
		{"Console", 2, ICON_PROGRAM},
		{"Notes", 3, ICON_PROGRAM},
		{"Photos", 4, ICON_PROGRAM},
		{"Zebre", 5, ICON_PROGRAM}
	};

	luna_lm_init(&g_m);
	luna_lm_set_apps(&g_m, apps, 6);
}

static uint64_t	rendu(const t_lmodel *m)
{
	t_launcher	l;

	l.user = "Nom";
	l.tiles = NULL;
	l.ntiles = 0;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	lt_clear(&g_t, LC_BLACK);
	luna_launcher_view(&g_t.s, &l, m, g_panel);
	return (lt_hash(&g_t.s));
}

static void	vues_distinctes(void)
{
	uint64_t	epingle;
	uint64_t	recent;
	uint64_t	toutes;
	uint64_t	recherche;

	garnir();
	epingle = rendu(&g_m);
	luna_lm_set_tab(&g_m, LUNA_LV_RECENT);
	recent = rendu(&g_m);
	luna_lm_set_tab(&g_m, LUNA_LV_ALL);
	toutes = rendu(&g_m);
	luna_lm_type(&g_m, 'c');
	recherche = rendu(&g_m);
	h_true(epingle != recent && recent != toutes && toutes != recherche
		&& epingle != toutes, "quatre vues, quatre rendus");
	luna_lm_type(&g_m, 'o');
	h_true(rendu(&g_m) != recherche, "la saisie change le rendu");
	h_true(lt_guard_ok(&g_t), "aucun debordement");
}

static void	modele_nul(void)
{
	t_launcher	l;
	uint64_t	a;

	l.user = "Nom";
	l.tiles = NULL;
	l.ntiles = 0;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	lt_clear(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, g_panel);
	a = lt_hash(&g_t.s);
	h_true(rendu(NULL) == a, "sans modele : rendu historique identique");
}

static void	limites(void)
{
	t_launcher	l;
	uint64_t	blank;

	garnir();
	luna_lm_set_tab(&g_m, LUNA_LV_ALL);
	lt_clear(&g_t, LC_BLACK);
	blank = lt_hash(&g_t.s);
	l.user = NULL;
	l.tiles = NULL;
	l.ntiles = 0;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	luna_launcher_view(&g_t.s, &l, &g_m, lp_rect(12, 20, 319, 289));
	h_true(lt_hash(&g_t.s) == blank, "largeur minimale - 1 : rien");
	luna_launcher_view(&g_t.s, &l, &g_m, lp_rect(12, 20, 540, 288));
	h_true(lt_hash(&g_t.s) == blank, "hauteur 288 : rien");
	luna_launcher_view(&g_t.s, NULL, &g_m, g_panel);
	h_true(lt_hash(&g_t.s) == blank, "lanceur NULL : rien");
}

static void	geometrie(void)
{
	t_launcherlayout	l;
	t_rect				a;
	t_rect				b;
	int32_t				i;

	luna_launcher_layout(g_panel, 4, &l);
	i = 0;
	while (i + 1 < LUNA_LM_ROWS)
	{
		a = luna_launcher_row_rect(&l, i);
		b = luna_launcher_row_rect(&l, i + 1);
		h_eq_i64("lignes jointives", a.y + a.h, b.y);
		i++;
	}
	b = luna_launcher_row_rect(&l, LUNA_LM_ROWS - 1);
	h_true(b.y + b.h <= l.sep.y, "derniere ligne au-dessus du filet");
	h_true(luna_launcher_row_rect(&l, LUNA_LM_ROWS).w == 0, "ligne hors plage");
	h_true(luna_launcher_row_rect(&l, -1).w == 0, "ligne negative");
}

static void	hits(void)
{
	t_launcherlayout	l;
	t_rect				r;

	luna_launcher_layout(g_panel, 4, &l);
	r = luna_launcher_row_rect(&l, 2);
	h_eq_i64("coin haut gauche ligne 2", luna_launcher_row_hit(&l, r.x, r.y), 2);
	h_eq_i64("dernier pixel ligne 2",
		luna_launcher_row_hit(&l, r.x + r.w - 1, r.y + r.h - 1), 2);
	h_eq_i64("juste a droite", luna_launcher_row_hit(&l, r.x + r.w, r.y),
		LUNA_LM_NONE);
	h_eq_i64("juste au-dessus des lignes",
		luna_launcher_row_hit(&l, r.x, luna_launcher_row_rect(&l, 0).y - 1),
		LUNA_LM_NONE);
	r = luna_launcher_tab_rect(&l, LUNA_LV_RECENT);
	h_eq_i64("onglet recents", luna_launcher_tab_hit(&l, r.x + 1, r.y + 1),
		LUNA_LV_RECENT);
	r = luna_launcher_tab_rect(&l, LUNA_LV_ALL);
	h_eq_i64("onglet toutes", luna_launcher_tab_hit(&l, r.x + r.w - 1,
			r.y + r.h - 1), LUNA_LV_ALL);
	h_eq_i64("hors onglets", luna_launcher_tab_hit(&l, r.x + r.w, r.y),
		LUNA_LM_NONE);
	h_eq_i64("NULL", luna_launcher_tab_hit(NULL, 0, 0), LUNA_LM_NONE);
	h_true(luna_launcher_tab_rect(&l, 3).w == 0, "onglet 3 inexistant");
}

int	main(void)
{
	if (lt_open(&g_t, 640, 480) != 0)
		return (1);
	h_begin("a17/lanceur_recherche_rendu");
	h_run("vues Epinglees, Recents, Toutes, Recherche", vues_distinctes);
	h_run("modele NULL : repli", modele_nul);
	h_run("tailles minimales : valeurs limites", limites);
	h_run("lignes : geometrie", geometrie);
	h_run("clics : onglets et lignes", hits);
	lt_close(&g_t);
	return (h_end());
}
