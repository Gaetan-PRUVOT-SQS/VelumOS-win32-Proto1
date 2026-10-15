#include "lt_gal.h"

void	lt_gal_text(t_surface *s, t_rect box, const char *txt, t_color c)
{
	t_ltext	t;

	t.font = FONT_UI;
	t.color = c;
	t.shadow = 0;
	t.text = txt;
	t.box = box;
	t.align = LA_CENTER;
	lp_text(s, &t);
}

static void	gal_win(t_surface *s, t_lunawin *w)
{
	t_rect	c;

	luna_window_frame(s, w);
	c = luna_window_client(w);
	lp_fill(s, c, luna_color_window());
	lt_gal_text(s, c, w->title, luna_color_text());
}

static void	gal_win_a(t_surface *s)
{
	t_lunawin	w;

	w = lt_win(lp_rect(10, 10, 300, 110), WS_DEFAULT, false);
	w.title = "Fen\xc3\xaatre active";
	gal_win(s, &w);
	w.outer.x = 330;
	w.active = false;
	w.title = "Fen\xc3\xaatre inactive";
	gal_win(s, &w);
	w.outer.x = 650;
	w.active = true;
	w.hot = HT_MINBUTTON;
	w.pressed = HT_CLOSE;
	w.title = "Survol et appui";
	gal_win(s, &w);
}

static void	gal_win_b(t_surface *s)
{
	t_lunawin	w;

	w = lt_win(lp_rect(10, 140, 420, 70), WS_DEFAULT, true);
	w.title = "Maximis\xc3\xa9" "e";
	w.hot = HT_MAXBUTTON;
	gal_win(s, &w);
	w = lt_win(lp_rect(450, 140, 220, 80), WS_CAPTION | WS_SYSMENU
			| WS_TOOLWINDOW, false);
	w.title = "Outil";
	gal_win(s, &w);
	w = lt_win(lp_rect(690, 140, 300, 90), WS_CAPTION | WS_SYSMENU
			| WS_MINBOX, false);
	w.title = "Un titre tres long qui ne tient pas dans la barre";
	gal_win(s, &w);
}

void	lt_gal_windows(t_surface *s)
{
	gal_win_a(s);
	gal_win_b(s);
}
