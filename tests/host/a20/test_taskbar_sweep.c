#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "layout.h"
#include "taskbar.h"

static void	check_row(const t_rect *r, int32_t shown, t_rect strip)
{
	int32_t	i;

	i = 0;
	while (i < shown)
	{
		h_true(r[i].w >= TB_BTN_MIN_W && r[i].w <= TB_BTN_MAX_W, "largeur");
		h_true(lay_inside(strip, r[i]), "dans la bande");
		if (i > 0)
			h_true(r[i].x == r[i - 1].x + r[i - 1].w + TB_GAP, "contigus");
		i++;
	}
	h_true(r[0].x == strip.x, "premier a gauche");
}

static void	sweep_invariants_de_1_a_200(void)
{
	t_rect		out[TASK_MAX];
	t_rect		strip;
	int32_t		n;
	int32_t		shown;
	int32_t		end;

	strip = lay_rect(74, 20, 732, 40);
	n = 1;
	while (n <= 200)
	{
		shown = tb_layout(strip, (uint32_t)n, out);
		h_true(shown >= 1 && shown <= n && shown <= TASK_MAX, "nombre montre");
		check_row(out, shown, strip);
		end = out[shown - 1].x + out[shown - 1].w;
		h_true(end <= strip.x + strip.w, "ne deborde pas");
		if (out[0].w < TB_BTN_MAX_W)
			h_eq_i64("largeur totale constante", end, strip.x + strip.w);
		n++;
	}
}

static void	sweep_largeur_decroissante(void)
{
	t_rect	out[TASK_MAX];
	t_rect	strip;
	int32_t	n;
	int32_t	prev;

	strip = lay_rect(0, 0, 732, 40);
	prev = TB_BTN_MAX_W;
	n = 1;
	while (n <= 30)
	{
		tb_layout(strip, (uint32_t)n, out);
		h_true(out[0].w <= prev, "la largeur ne grandit pas");
		prev = out[0].w;
		n++;
	}
	h_eq_i64("16 boutons tiennent: (732+6)/(40+6)", tb_layout(strip, 16, out),
		16);
	h_eq_i64("au dela, les derniers sont masques", tb_layout(strip, 60, out),
		16);
}

static void	sweep_frontiere_du_plafond(void)
{
	t_rect	out[TASK_MAX];
	int32_t	w;

	w = 3 * TB_BTN_MAX_W + 2 * TB_GAP;
	h_eq_i64("pile 3 boutons au plafond", tb_layout(lay_rect(0, 0, w, 40), 3,
			out), 3);
	h_eq_i64("plafond atteint", out[2].w, TB_BTN_MAX_W);
	h_eq_i64("un pixel de plus", tb_layout(lay_rect(0, 0, w + 1, 40), 3, out),
		3);
	h_true(out[0].w <= TB_BTN_MAX_W && out[1].w <= TB_BTN_MAX_W
		&& out[2].w <= TB_BTN_MAX_W, "jamais plus large que le plafond");
	h_eq_i64("un pixel de moins", tb_layout(lay_rect(0, 0, w - 1, 40), 3,
			out), 3);
	h_true(out[0].w == TB_BTN_MAX_W && out[2].w == TB_BTN_MAX_W - 1,
		"le pixel manquant va aux derniers");
}

int	main(void)
{
	h_begin("a20/taskbar-sweep");
	h_run("barre: invariants de 1 a 200", sweep_invariants_de_1_a_200);
	h_run("barre: largeur decroissante", sweep_largeur_decroissante);
	h_run("barre: frontiere du plafond", sweep_frontiere_du_plafond);
	return (h_end());
}
