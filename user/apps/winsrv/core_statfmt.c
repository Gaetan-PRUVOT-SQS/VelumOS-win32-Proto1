#include "ws_core.h"

typedef struct s_wline
{
	char		*buf;
	uint32_t	cap;
	uint32_t	len;
}	t_wline;

static void	put_char(t_wline *l, char c)
{
	if (l->len + 1 < l->cap)
	{
		l->buf[l->len] = c;
		l->len++;
	}
	l->buf[l->len] = '\0';
}

static void	put_str(t_wline *l, const char *s)
{
	while (*s != '\0')
	{
		put_char(l, *s);
		s++;
	}
}

static void	put_num(t_wline *l, uint64_t v)
{
	char		digits[20];
	uint32_t	n;

	n = 0;
	digits[n++] = (char)('0' + v % 10);
	v /= 10;
	while (v != 0 && n < sizeof(digits))
	{
		digits[n++] = (char)('0' + v % 10);
		v /= 10;
	}
	while (n > 0)
	{
		n--;
		put_char(l, digits[n]);
	}
}

static void	put_field(t_wline *l, const char *name, uint64_t v)
{
	put_str(l, name);
	put_num(l, v);
}

uint32_t	wst_format(const t_wmeter *m, char *buf, uint32_t n)
{
	t_wline	l;

	if (buf == NULL || n == 0)
		return (0);
	l.buf = buf;
	l.cap = n;
	l.len = 0;
	buf[0] = '\0';
	put_field(&l, "winsrv: mesure images=", m->window_frames);
	put_field(&l, " rects=", m->window.rects);
	put_field(&l, " sales=", m->window.px_dirty);
	put_field(&l, " composes=", m->window.px_comp);
	put_field(&l, " peints=", m->window.px_paint);
	put_field(&l, " floutes=", m->window.px_glass);
	put_field(&l, " ns=", m->window.ns);
	put_field(&l, " max=", m->ns_max);
	put_field(&l, " repli=", m->window.overflow);
	return (l.len);
}
