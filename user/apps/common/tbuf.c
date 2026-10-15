#include "velum/err.h"
#include "tbuf.h"

void	tb_init(t_tbuf *b, char *p, size_t size)
{
	b->p = p;
	b->size = size;
	b->len = 0;
	b->over = 0;
	if (size)
		p[0] = '\0';
}

void	tb_putc(t_tbuf *b, char c)
{
	if (b->size == 0 || b->len + 1 >= b->size)
	{
		b->over = 1;
		return ;
	}
	b->p[b->len] = c;
	b->len++;
	b->p[b->len] = '\0';
}

void	tb_str(t_tbuf *b, const char *s)
{
	while (*s)
	{
		tb_putc(b, *s);
		s++;
	}
}

void	tb_num(t_tbuf *b, uint64_t v, int width)
{
	char	d[TBUF_DIGITS_MAX];
	int		n;

	n = 0;
	while (v || n == 0)
	{
		d[n] = (char)('0' + v % 10);
		v /= 10;
		n++;
	}
	while (n < width && n < TBUF_DIGITS_MAX)
	{
		d[n] = '0';
		n++;
	}
	while (n > 0)
	{
		n--;
		tb_putc(b, d[n]);
	}
}

int	tb_result(const t_tbuf *b)
{
	if (b->over)
		return (E_RANGE);
	return ((int)b->len);
}
