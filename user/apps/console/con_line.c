#include "velum/libk.h"
#include "velum/err.h"
#include "console.h"

static int	is_cont(char c)
{
	return (((uint8_t)c & 0xc0) == 0x80);
}

void	line_init(t_conline *l)
{
	memset(l, 0, sizeof(*l));
}

uint32_t	line_cells(const char *s, uint32_t bytes)
{
	uint32_t	n;
	uint32_t	i;

	n = 0;
	i = 0;
	while (i < bytes)
	{
		if (!is_cont(s[i]))
			n++;
		i++;
	}
	return (n);
}

uint32_t	utf8_put(uint32_t cp, char *out)
{
	if (cp < 0x80)
		out[0] = (char)cp;
	else if (cp < 0x800)
	{
		out[0] = (char)(0xc0 | (cp >> 6));
		out[1] = (char)(0x80 | (cp & 0x3f));
		return (2);
	}
	else if (cp < 0x10000 && (cp < 0xd800 || cp > 0xdfff))
	{
		out[0] = (char)(0xe0 | (cp >> 12));
		out[1] = (char)(0x80 | ((cp >> 6) & 0x3f));
		out[2] = (char)(0x80 | (cp & 0x3f));
		return (3);
	}
	else if (cp >= 0x10000 && cp <= 0x10ffff)
	{
		out[0] = (char)(0xf0 | (cp >> 18));
		out[1] = (char)(0x80 | ((cp >> 12) & 0x3f));
		out[2] = (char)(0x80 | ((cp >> 6) & 0x3f));
		out[3] = (char)(0x80 | (cp & 0x3f));
		return (4);
	}
	else
		return (0);
	return (1);
}

int	line_insert(t_conline *l, const char *utf8, uint32_t n)
{
	if (n == 0 || l->len + n >= CON_LINE_MAX)
		return (E_NOSPC);
	memmove(l->buf + l->cur + n, l->buf + l->cur, l->len - l->cur);
	memcpy(l->buf + l->cur, utf8, n);
	l->len += n;
	l->cur += n;
	l->buf[l->len] = '\0';
	l->hist_pos = l->hist_n;
	return (0);
}

static uint32_t	prev_start(const t_conline *l)
{
	uint32_t	i;

	i = l->cur;
	while (i > 0)
	{
		i--;
		if (!is_cont(l->buf[i]))
			break ;
	}
	return (i);
}

static uint32_t	next_end(const t_conline *l)
{
	uint32_t	i;

	i = l->cur;
	if (i < l->len)
		i++;
	while (i < l->len && is_cont(l->buf[i]))
		i++;
	return (i);
}

void	line_backspace(t_conline *l)
{
	uint32_t	from;

	if (l->cur == 0)
		return ;
	from = prev_start(l);
	memmove(l->buf + from, l->buf + l->cur, l->len - l->cur);
	l->len -= l->cur - from;
	l->cur = from;
	l->buf[l->len] = '\0';
}

void	line_delete(t_conline *l)
{
	uint32_t	to;

	if (l->cur >= l->len)
		return ;
	to = next_end(l);
	memmove(l->buf + l->cur, l->buf + to, l->len - to);
	l->len -= to - l->cur;
	l->buf[l->len] = '\0';
}

void	line_left(t_conline *l)
{
	l->cur = prev_start(l);
}

void	line_right(t_conline *l)
{
	l->cur = next_end(l);
}

void	line_home(t_conline *l)
{
	l->cur = 0;
}

void	line_end(t_conline *l)
{
	l->cur = l->len;
}

void	line_kill_end(t_conline *l)
{
	l->len = l->cur;
	l->buf[l->len] = '\0';
}

void	line_kill_all(t_conline *l)
{
	l->len = 0;
	l->cur = 0;
	l->buf[0] = '\0';
	l->hist_pos = l->hist_n;
}

static void	load_text(t_conline *l, const char *text)
{
	l->len = (uint32_t)strlen(text);
	memcpy(l->buf, text, l->len + 1);
	l->cur = l->len;
}

void	line_hist_prev(t_conline *l)
{
	if (l->hist_n == 0 || l->hist_pos == 0)
		return ;
	if (l->hist_pos == l->hist_n)
		memcpy(l->draft, l->buf, l->len + 1);
	l->hist_pos--;
	load_text(l, l->hist[l->hist_pos]);
}

void	line_hist_next(t_conline *l)
{
	if (l->hist_pos >= l->hist_n)
		return ;
	l->hist_pos++;
	if (l->hist_pos == l->hist_n)
		load_text(l, l->draft);
	else
		load_text(l, l->hist[l->hist_pos]);
}

static void	hist_push(t_conline *l)
{
	if (l->len == 0)
		return ;
	if (l->hist_n > 0 && strcmp(l->hist[l->hist_n - 1], l->buf) == 0)
		return ;
	if (l->hist_n == CON_HIST_MAX)
	{
		memmove(l->hist[0], l->hist[1], (CON_HIST_MAX - 1) * CON_LINE_MAX);
		l->hist_n--;
	}
	memcpy(l->hist[l->hist_n], l->buf, l->len + 1);
	l->hist_n++;
}

void	line_commit(t_conline *l, char *out)
{
	memcpy(out, l->buf, l->len + 1);
	hist_push(l);
	l->len = 0;
	l->cur = 0;
	l->buf[0] = '\0';
	l->draft[0] = '\0';
	l->hist_pos = l->hist_n;
}
