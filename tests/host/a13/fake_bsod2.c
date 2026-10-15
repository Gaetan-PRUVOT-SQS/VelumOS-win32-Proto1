#include "kfix.h"

static bool	utf8_clean(const char *s, int32_t len)
{
	const char	*p;
	const char	*end;

	p = s;
	end = s + len;
	while (p < end)
	{
		if (font_utf8_next(&p, end) == 0xfffd)
			return (false);
	}
	return (true);
}

static int	check_line(const t_bsod_line *ln, const char *text, int32_t cols)
{
	int	bad;

	bad = 0;
	bad += (ln->s < text || ln->len < 0);
	bad += (font_text_width(NULL, ln->s, ln->len) > cols * g_ffont.cell_w);
	return (bad);
}

int	fake_wrapck(const t_bsod_wrap *w, const char *t, int32_t c)
{
	int32_t	i;
	int		bad;

	bad = (w->count > w->max || w->count < 0);
	i = 0;
	while (i < w->count)
	{
		bad += check_line(&w->lines[i], t, c);
		if (i > 0)
			bad += (w->lines[i].s < w->lines[i - 1].s + w->lines[i - 1].len);
		i++;
	}
	return (bad);
}

int	fake_wrapu8(const t_bsod_wrap *w)
{
	int32_t	i;
	int		bad;

	i = 0;
	bad = 0;
	while (i < w->count)
	{
		bad += !utf8_clean(w->lines[i].s, w->lines[i].len);
		i++;
	}
	return (bad);
}
