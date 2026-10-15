#include "kcon_int.h"

void	kcon_newline(t_kcon *k)
{
	k->col = 0;
	k->row++;
	if (k->row < k->rows)
		return ;
	kcon_scroll(k);
	k->row = k->rows - 1;
}

void	kcon_backspace(t_kcon *k)
{
	if (k->col > 0)
		k->col--;
}

void	kcon_tab(t_kcon *k)
{
	int32_t	stop;

	if (k->col >= k->cols)
		kcon_newline(k);
	stop = (k->col / KCON_TAB + 1) * KCON_TAB;
	if (stop > k->cols)
		stop = k->cols;
	while (k->col < stop)
	{
		kcon_cell_draw(k, " ", 1);
		k->col++;
	}
}

void	kcon_put(t_kcon *k, const char *text, int32_t len, uint32_t cp)
{
	if (cp == '\n')
		kcon_newline(k);
	else if (cp == '\r')
		k->col = 0;
	else if (cp == '\b')
		kcon_backspace(k);
	else if (cp == '\t')
		kcon_tab(k);
	else if (cp >= 0x20 && !(cp >= 0x7f && cp < 0xa0))
	{
		if (k->col >= k->cols)
			kcon_newline(k);
		kcon_cell_draw(k, text, len);
		k->col++;
	}
}

void	kcon_feed(t_kcon *k, const char *s, size_t n)
{
	const char	*p;
	const char	*end;
	const char	*start;
	uint32_t	cp;

	p = s;
	end = s + n;
	while (p < end)
	{
		start = p;
		cp = font_utf8_next(&p, end);
		if (p <= start)
			p = start + 1;
		if (p > end)
			p = end;
		kcon_put(k, start, (int32_t)(p - start), cp);
	}
}
