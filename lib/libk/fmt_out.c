#include "fmt_int.h"

void	out_init(t_out *o, char *buf, size_t size)
{
	o->buf = buf;
	o->size = size;
	if (!buf)
		o->size = 0;
	o->pos = 0;
}

void	out_putc(t_out *o, char c)
{
	if (o->size && o->pos < o->size - 1)
		o->buf[o->pos] = c;
	o->pos++;
}

void	out_fill(t_out *o, char c, int64_t n)
{
	size_t	room;
	size_t	i;

	if (n <= 0)
		return ;
	room = 0;
	if (o->size && o->pos < o->size - 1)
		room = o->size - 1 - o->pos;
	if ((uint64_t)n < room)
		room = (size_t)n;
	i = 0;
	while (i < room)
		o->buf[o->pos + i++] = c;
	o->pos += (size_t)n;
}

void	out_write(t_out *o, const char *s, size_t n)
{
	while (n--)
		out_putc(o, *s++);
}

void	out_term(t_out *o)
{
	size_t	i;

	if (!o->size)
		return ;
	i = o->pos;
	if (i >= o->size)
		i = o->size - 1;
	o->buf[i] = '\0';
}
