#include "help.h"

static void	button(t_surface *s, const t_lunawin *w, uint32_t ht, int32_t k)
{
	t_lunametrics	m;
	t_rect			r;
	t_color			c;

	luna_metrics(&m);
	r = rect_make(w->outer.x + w->outer.w - m.frame_w - (m.btn_w + 2) * k,
			w->outer.y + (m.caption_h - m.btn_h) / 2, m.btn_w, m.btn_h);
	c = 0xff2050c0;
	if (w->hot == ht)
		c = 0xff60a0ff;
	if (w->pressed == ht)
		c = 0xff102060;
	if (ht == HT_CLOSE)
		c = (c & 0xff00ffff) | 0x00e00000;
	gfx_fill(s, r, c);
}

__attribute__((weak)) void	luna_window_frame(t_surface *s,
	const t_lunawin *w)
{
	t_color	c;

	c = 0xff7a96df;
	if (w->active)
		c = 0xff0058ee;
	gfx_fill(s, w->outer, c);
	gfx_fill(s, luna_window_client(w), 0xffece9d8);
	if (w->style & WS_SYSMENU)
		button(s, w, HT_CLOSE, 1);
	if (w->style & WS_MAXBOX)
		button(s, w, HT_MAXBUTTON, 2);
	if (w->style & WS_MINBOX)
		button(s, w, HT_MINBUTTON, 3);
}
