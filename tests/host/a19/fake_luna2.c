#include "fake.h"

void	luna_checkbox(t_surface *s, t_point p, t_lunastate st, bool on)
{
	t_rect	r;

	r = rect_make(p.x, p.y, 13, 13);
	fake_log_add(s, FK_CHECK, r, (int)st);
	fake_log_b((int)on);
	gfx_fill(s, r, 0xff00c000u | (uint32_t)st);
}

void	luna_radio(t_surface *s, t_point p, t_lunastate st, bool on)
{
	t_rect	r;

	r = rect_make(p.x, p.y, 13, 13);
	fake_log_add(s, FK_RADIO, r, (int)st);
	fake_log_b((int)on);
	gfx_fill(s, r, 0xff0000c0u | (uint32_t)st);
}

void	luna_edit_frame(t_surface *s, t_rect r, bool enabled)
{
	fake_log_add(s, FK_EDIT_FRAME, r, (int)enabled);
	gfx_hline(s, (t_point){r.x, r.y}, r.w, 0xff7f9db9);
	gfx_hline(s, (t_point){r.x, r.y + r.h - 1}, r.w, 0xff7f9db9);
	gfx_vline(s, (t_point){r.x, r.y}, r.h, 0xff7f9db9);
	gfx_vline(s, (t_point){r.x + r.w - 1, r.y}, r.h, 0xff7f9db9);
}

void	luna_groupbox(t_surface *s, t_rect r, int32_t label_w)
{
	fake_log_add(s, FK_GROUP, r, (int)label_w);
	gfx_hline(s, (t_point){r.x, r.y + 5}, r.w, 0xffd0d0bf);
}

void	luna_progress(t_surface *s, t_rect r, uint32_t pct)
{
	fake_log_add(s, FK_PROGRESS, r, (int)pct);
	gfx_fill(s, r, 0xff00a000);
}
