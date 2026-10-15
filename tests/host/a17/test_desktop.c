#include "harness.h"
#include "lt_gal.h"

static void	desktop_render(void)
{
	t_lt		t;
	t_lunawin	w;

	h_true(lt_open(&t, 1024, 768) == 0, "allocation");
	lt_gal_desktop(&t.s, lp_rect(0, 0, 1024, 768));
	w = lt_win(lp_rect(300, 120, 520, 330), WS_DEFAULT, false);
	w.title = "Poste de travail";
	w.icon = ICON_COMPUTER;
	luna_window_frame(&t.s, &w);
	lp_fill(&t.s, luna_window_client(&w), luna_color_window());
	lt_out_dir();
	h_true(lt_png("build/a17/bureau.png", &t.s, 1) == 0, "ecriture png");
	lt_zoom(&t.s, lp_rect(0, 718, 400, 50), 3, "build/a17/zoom_barre.png");
	lt_zoom(&t.s, lp_rect(900, 718, 124, 50), 4, "build/a17/zoom_tray.png");
	lt_zoom(&t.s, lp_rect(0, 270, 400, 100), 3, "build/a17/zoom_menu_haut.png");
	lt_zoom(&t.s, lp_rect(0, 600, 400, 118), 3, "build/a17/zoom_menu_bas.png");
	lt_close(&t);
}

static void	logon_render(void)
{
	t_lt	t;

	h_true(lt_open(&t, 1024, 768) == 0, "allocation");
	luna_logon_bg(&t.s, lp_rect(0, 0, 1024, 768));
	lt_png("build/a17/logon.png", &t.s, 1);
	lt_close(&t);
}

static void	boot_render(void)
{
	t_lt	t;

	h_true(lt_open(&t, 1024, 768) == 0, "allocation");
	luna_boot_draw(&t.s, 40, 5);
	lt_png("build/a17/boot.png", &t.s, 1);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/desktop");
	h_run("bureau", desktop_render);
	h_run("fond de session", logon_render);
	h_run("ecran de demarrage", boot_render);
	return (h_end());
}
