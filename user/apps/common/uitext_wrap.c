#include "velum/libk.h"
#include "uitext.h"

static size_t	break_pos(t_fontid font, const char *text, int32_t max_w)
{
	char	buf[UI_WRAP_MAX];
	size_t	i;
	size_t	best;

	best = 0;
	i = 0;
	while (text[i] && i < sizeof(buf) - 1)
	{
		if (text[i] == ' ')
		{
			memcpy(buf, text, i);
			buf[i] = '\0';
			if (ui_text_width(font, buf) <= max_w)
				best = i;
		}
		i++;
	}
	return (best);
}

void	ui_text_wrap(t_surface *s, const t_uitext *st, const char *text)
{
	char		first[UI_WRAP_MAX];
	t_uitext	next;
	size_t		cut;

	cut = break_pos(st->font, text, st->max_w);
	if (cut == 0 || ui_text_width(st->font, text) <= st->max_w)
	{
		ui_text_shadow(s, st, text);
		return ;
	}
	memcpy(first, text, cut);
	first[cut] = '\0';
	ui_text_shadow(s, st, first);
	next = *st;
	next.at.y += ui_font_height(st->font) + 4;
	ui_text_shadow(s, &next, text + cut + 1);
}
