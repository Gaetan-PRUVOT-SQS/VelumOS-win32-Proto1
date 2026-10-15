#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

#define BACKDROP 0xff112233u

static void	band_model(void)
{
	t_fakeui	u;
	t_rect		r;
	t_rect		all;

	fake_begin(&u, 80, 60);
	fake_px_fill(&u.s, BACKDROP);
	r = rect_make(10, 8, 30, 20);
	all = rect_make(0, 0, 80, 60);
	ctl_focus_ring(&u.s, r);
	h_eq_i64("anneau : bande accent de 2 px sur le bord",
		fake_px_count(&u.s, r, luna_color_accent()), fake_band_area(r));
	h_eq_i64("anneau : rien hors du rectangle",
		fake_px_count(&u.s, all, luna_color_accent()), fake_band_area(r));
	h_eq_i64("anneau : jeu de 2 px et face intacts",
		fake_px_count(&u.s, rect_make(12, 10, 26, 16), BACKDROP), 26 * 16);
	h_eq_i64("anneau : plus de pointille noir",
		fake_px_count(&u.s, all, 0xff000000), 0);
	fake_done(&u);
}

static void	limits(void)
{
	t_fakeui	u;
	int			sizes[5];
	int			i;
	t_rect		r;

	fake_begin(&u, 40, 40);
	sizes[0] = 1;
	sizes[1] = 3;
	sizes[2] = 4;
	sizes[3] = 5;
	sizes[4] = 6;
	i = 0;
	while (i < 5)
	{
		fake_px_fill(&u.s, BACKDROP);
		r = rect_make(10, 10, sizes[i], sizes[i]);
		ctl_focus_ring(&u.s, r);
		h_eq_i64("valeurs limites : surface couverte selon la taille",
			fake_px_count(&u.s, rect_make(0, 0, 40, 40),
				luna_color_accent()), fake_band_area(r));
		i++;
	}
	fake_done(&u);
}

static void	empty_rects(void)
{
	t_fakeui	u;

	fake_begin(&u, 40, 40);
	fake_px_fill(&u.s, BACKDROP);
	ctl_focus_ring(&u.s, rect_make(10, 10, 0, 5));
	ctl_focus_ring(&u.s, rect_make(10, 10, 5, -3));
	h_eq_i64("rectangle vide ou negatif : rien dessine",
		fake_px_count(&u.s, rect_make(0, 0, 40, 40), BACKDROP), 40 * 40);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/ring");
	h_run("anneau : modele de bande 2 px (partition)", band_model);
	h_run("anneau : valeurs limites de taille", limits);
	h_run("anneau : rectangle vide ou negatif", empty_rects);
	return (h_end());
}
