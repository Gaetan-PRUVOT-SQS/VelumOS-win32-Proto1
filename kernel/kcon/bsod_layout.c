#include "kcon_int.h"
#include "velum/libk.h"

static const char	*next_cell(const char *p, const char *end)
{
	const char	*r;

	r = p;
	font_utf8_next(&r, end);
	if (r <= p)
		r = p + 1;
	if (r > end)
		r = end;
	return (r);
}

int32_t	bsod_cut(const char *s, int32_t len, int32_t cells)
{
	const char	*p;
	const char	*end;

	p = s;
	end = s + len;
	while (p < end && cells > 0)
	{
		p = next_cell(p, end);
		cells--;
	}
	return ((int32_t)(p - s));
}

static const char	*take_line(const char *p, const char *end, int32_t cols,
		t_bsod_line *out)
{
	const char	*q;
	const char	*brk;
	int32_t		n;

	q = p;
	brk = NULL;
	n = 0;
	while (q < end && n < cols && *q != '\n')
	{
		if (*q == ' ')
			brk = q;
		q = next_cell(q, end);
		n++;
	}
	if (q < end && *q != '\n' && *q != ' ' && brk && brk > p)
		q = brk;
	out->s = p;
	out->len = (int32_t)(q - p);
	if (q < end && (*q == '\n' || *q == ' '))
		q++;
	return (q);
}

static void	cut_last(t_bsod_wrap *w)
{
	t_bsod_line	*ln;
	int32_t		keep;

	keep = w->cols - BSOD_DOTS;
	if (keep < 0)
		keep = 0;
	ln = &w->lines[w->count - 1];
	ln->len = bsod_cut(ln->s, ln->len, keep);
}

void	bsod_wrap(t_bsod_wrap *w)
{
	const char	*p;
	const char	*end;

	w->count = 0;
	w->truncated = false;
	if (!w->text || w->cols < 1 || w->max < 1)
		return ;
	if (w->max > BSOD_LINES_MAX)
		w->max = BSOD_LINES_MAX;
	p = w->text;
	end = p + strlen(p);
	while (p < end && w->count < w->max)
	{
		p = take_line(p, end, w->cols, &w->lines[w->count]);
		w->count++;
	}
	while (p < end && (*p == ' ' || *p == '\n'))
		p++;
	w->truncated = (p < end);
	if (w->truncated)
		cut_last(w);
}
