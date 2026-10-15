#include <stdio.h>
#include "harness.h"
#include "lt_gal.h"

static void	gallery_render(void)
{
	t_lt	t;

	h_true(lt_open(&t, 1024, 768) == 0, "allocation");
	lt_clear(&t, luna_color_window());
	lt_gal_windows(&t.s);
	lt_gal_buttons(&t.s, 10, 250);
	lt_gal_checks(&t.s, 10, 290);
	lt_gal_scrolls(&t.s, 270, 280);
	lt_gal_misc(&t.s, 580, 250);
	lt_gal_icons(&t.s, 10, 440);
	lt_out_dir();
	h_true(lt_png("build/a17/galerie.png", &t.s, 1) == 0, "ecriture png");
	lt_zoom(&t.s, lp_rect(10, 10, 140, 50), 6, "build/a17/zoom_cadre_g.png");
	lt_zoom(&t.s, lp_rect(200, 10, 115, 50), 6, "build/a17/zoom_cadre_d.png");
	lt_zoom(&t.s, lp_rect(10, 245, 270, 100), 4, "build/a17/zoom_boutons.png");
	lt_zoom(&t.s, lp_rect(265, 275, 280, 160), 4, "build/a17/zoom_scroll.png");
	lt_zoom(&t.s, lp_rect(575, 245, 270, 190), 4, "build/a17/zoom_divers.png");
	lt_zoom(&t.s, lp_rect(5, 435, 360, 125), 3, "build/a17/zoom_icones1.png");
	lt_zoom(&t.s, lp_rect(360, 435, 380, 125), 3, "build/a17/zoom_icones2.png");
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/galerie");
	h_run("rendu de la galerie", gallery_render);
	return (h_end());
}
