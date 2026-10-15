#include "velum/libk.h"
#include "uitext.h"

void	ui_text(t_surface *s, const t_uitext *st, const char *text)
{
	t_textreq		rq;
	const t_font	*f;

	f = font_get(st->font);
	if (!f || !text)
		return ;
	rq.font = f;
	rq.at = st->at;
	rq.color = st->color;
	rq.text = text;
	rq.len = (int32_t)strlen(text);
	if (st->max_w > 0)
		rq.len = font_fit(f, text, st->max_w);
	font_draw(s, &rq);
}

void	ui_text_shadow(t_surface *s, const t_uitext *st, const char *text)
{
	t_uitext	back;

	back = *st;
	back.at.x++;
	back.at.y++;
	back.color = UI_SHADOW;
	ui_text(s, &back, text);
	ui_text(s, st, text);
}

int32_t	ui_text_width(t_fontid font, const char *text)
{
	return (font_text_width(font_get(font), text, -1));
}

int32_t	ui_font_height(t_fontid font)
{
	const t_font	*f;

	f = font_get(font);
	if (!f)
		return (0);
	return (f->height);
}
