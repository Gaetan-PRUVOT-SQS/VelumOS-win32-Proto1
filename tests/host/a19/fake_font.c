#include "fake.h"

static const t_font	g_fonts[3] = {
{9, 2, 11, 0, NULL, NULL, 0},
{9, 2, 11, 1, NULL, NULL, 0},
{19, 5, 24, 0, NULL, NULL, 0}
};

static int32_t	char_w(const t_font *f)
{
	if (f->nglyphs == 1)
		return (FAKE_BOLD_W);
	return (FAKE_CHAR_W);
}

const t_font	*font_get(t_fontid id)
{
	if (id == FONT_UI_BOLD)
		return (&g_fonts[1]);
	if (id == FONT_INS_BOLD_24)
		return (&g_fonts[2]);
	return (&g_fonts[0]);
}

int32_t	font_text_width(const t_font *f, const char *text, int32_t len)
{
	int32_t	i;
	int32_t	n;

	i = 0;
	n = 0;
	while (i < len && text[i])
	{
		if (((uint8_t)text[i] & 0xc0) != 0x80)
			n++;
		i++;
	}
	return (n * char_w(f));
}

int32_t	font_fit(const t_font *f, const char *text, int32_t max_w)
{
	int32_t	i;
	int32_t	n;

	i = 0;
	n = 0;
	while (text[i])
	{
		if (((uint8_t)text[i] & 0xc0) != 0x80)
		{
			if ((n + 1) * char_w(f) > max_w)
				break ;
			n++;
		}
		i++;
	}
	return (i);
}

void	font_draw(t_surface *dst, const t_textreq *rq)
{
	t_rect	r;

	r = rect_make(rq->at.x, rq->at.y,
			font_text_width(rq->font, rq->text, rq->len), rq->font->height);
	fake_log_add(dst, FK_TEXT, r, (int)rq->color);
	fake_log_b(rq->len);
	fake_log_text(rq->text, rq->len);
	gfx_fill(dst, rect_make(r.x, r.y, r.w, rq->font->ascent), rq->color);
}
