#include "lt_draw.h"

void	lt_d_frame_active(t_surface *s, t_rect r)
{
	t_lunawin	w;

	w = lt_win(r, WS_DEFAULT, false);
	w.title = "Titre";
	luna_window_frame(s, &w);
}

void	lt_d_frame_inactive(t_surface *s, t_rect r)
{
	t_lunawin	w;

	w = lt_win(r, WS_DEFAULT, false);
	w.active = false;
	w.hot = HT_CLOSE;
	w.pressed = HT_MINBUTTON;
	luna_window_frame(s, &w);
}

void	lt_d_frame_max(t_surface *s, t_rect r)
{
	t_lunawin	w;

	w = lt_win(r, WS_DEFAULT, true);
	w.hot = HT_MAXBUTTON;
	luna_window_frame(s, &w);
}

void	lt_d_frame_tool(t_surface *s, t_rect r)
{
	t_lunawin	w;

	w = lt_win(r, WS_CAPTION | WS_SYSMENU | WS_TOOLWINDOW, false);
	luna_window_frame(s, &w);
}

void	lt_d_frame_dialog(t_surface *s, t_rect r)
{
	t_lunawin	w;

	w = lt_win(r, WS_CAPTION | WS_SYSMENU | WS_MINBOX, false);
	w.title = "Un titre long, tres long, trop long pour tenir";
	luna_window_frame(s, &w);
}
