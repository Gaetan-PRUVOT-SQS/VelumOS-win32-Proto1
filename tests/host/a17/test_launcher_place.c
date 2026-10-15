#include "harness.h"
#include "velum/luna_launcher.h"
#include "lt.h"
#include "luna_int.h"

typedef struct s_placecase
{
	int32_t	sw;
	int32_t	sh;
	int32_t	bar;
	int32_t	w;
	int32_t	h;
	int32_t	y;
}	t_placecase;

static const t_placecase	g_case[] = {
{1280, 800, 80, 540, 289, 431}, {1024, 768, 80, 540, 289, 399},
{564, 600, 80, 540, 289, 231}, {563, 600, 80, 539, 289, 231},
{552, 600, 80, 528, 289, 231}, {320 + 24, 600, 80, 320, 289, 231},
{24, 600, 80, 0, 289, 231}, {0, 600, 80, 0, 289, 231},
{-5, 600, 80, 0, 289, 231}, {800, 369, 80, 540, 289, 0},
{800, 368, 80, 540, 288, 0}, {800, 80, 80, 540, 0, 0},
{800, 50, 80, 540, 0, 0}, {800, 600, -10, 540, 289, 311}};

static void	place_table(void)
{
	t_rect		r;
	size_t		i;

	i = 0;
	while (i < sizeof(g_case) / sizeof(g_case[0]))
	{
		r = luna_launcher_place_in(g_case[i].sw, g_case[i].sh, g_case[i].bar);
		h_eq_i64("x = 12", r.x, LUNA_LAUNCHER_MARGIN_X);
		h_eq_i64("largeur : 540 au plus, ecran moins 2 x 12",
			r.w, g_case[i].w);
		h_eq_i64("hauteur : 289 au plus, ecran moins la barre",
			r.h, g_case[i].h);
		h_eq_i64("bas du panneau a la barre ou au bord haut",
			r.y, g_case[i].y);
		i++;
	}
	r = luna_launcher_place_in(1280, 800, 80);
	h_eq_i64("meme x que luna_launcher_place",
		r.x, luna_launcher_place(800, 80).x);
	h_eq_i64("meme y que luna_launcher_place",
		r.y, luna_launcher_place(800, 80).y);
}

static void	screen_guard(void)
{
	t_lt		t;
	t_launcher	l;
	int32_t		w;

	l.user = "Nom";
	l.tiles = NULL;
	l.ntiles = 0;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_LOGOFF;
	w = 545;
	while (w <= 560)
	{
		h_true(lt_open(&t, w, 320) == 0, "allocation");
		lt_clear(&t, LC_BLACK);
		luna_launcher(&t.s, &l, lp_rect(12, 20, 540, 289));
		h_true(lt_guard_ok(&t), "largeur d'ecran 552 +- 7 : zones de garde");
		lt_close(&t);
		w++;
	}
}

int	main(void)
{
	h_begin("a17/lanceur_largeur_ecran");
	h_run("placement selon la largeur et la hauteur d'ecran", place_table);
	h_run("ecran juste trop etroit : aucune ecriture hors surface",
		screen_guard);
	return (h_end());
}
