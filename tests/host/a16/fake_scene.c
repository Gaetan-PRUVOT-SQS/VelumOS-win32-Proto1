#include "a16_test.h"

t_rect	rect_of(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

void	fill_rect(t_surface *s, t_rect r, t_color c)
{
	int32_t	x;
	int32_t	y;

	y = r.y;
	while (y < r.y + r.h)
	{
		x = r.x;
		while (x < r.x + r.w)
			gfx_put(s, point_of(x++, y), c);
		y++;
	}
}

void	ellipsis(t_surface *s, t_rect box, t_fontid id, const char *t)
{
	t_textreq	rq;
	int32_t		dots;

	req_set(&rq, id, point_of(box.x, box.y), t);
	dots = font_text_width(rq.font, "\xE2\x80\xA6", -1);
	rq.color = FAKE_INK;
	if (font_text_width(rq.font, t, -1) > box.w)
		rq.len = font_fit(rq.font, t, box.w - dots);
	font_draw(s, &rq);
	if (rq.len < 0)
		return ;
	rq.at.x += font_text_width(rq.font, t, rq.len);
	rq.text = "\xE2\x80\xA6";
	rq.len = -1;
	font_draw(s, &rq);
}

void	button_draw(t_surface *s, t_rect r, const char *label)
{
	int32_t		w;
	t_point		at;
	t_textreq	rq;

	fill_rect(s, r, 0xFF003C74);
	fill_rect(s, rect_of(r.x + 1, r.y + 1, r.w - 2, r.h - 2), 0xFFF4F3EE);
	w = font_text_width(font_get(FONT_UI), label, -1);
	at = point_of(r.x + (r.w - w) / 2,
			r.y + (r.h - font_get(FONT_UI)->height) / 2);
	req_set(&rq, FONT_UI, at, label);
	font_draw(s, &rq);
}
