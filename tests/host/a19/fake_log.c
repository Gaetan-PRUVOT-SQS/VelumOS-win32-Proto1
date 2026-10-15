#include <string.h>
#include "fake.h"

t_fakelog	g_log;

void	fake_log_clear(void)
{
	g_log.n = 0;
}

void	fake_log_add(const t_surface *s, int kind, t_rect r, int a)
{
	t_fakecall	*c;

	if (g_log.n >= FK_MAX)
		return ;
	c = &g_log.calls[g_log.n];
	g_log.n++;
	memset(c, 0, sizeof(*c));
	c->kind = kind;
	c->r = r;
	c->clip = s->clip;
	c->a = a;
}

void	fake_log_b(int b)
{
	if (g_log.n)
		g_log.calls[g_log.n - 1].b = b;
}

void	fake_log_text(const char *text, int len)
{
	t_fakecall	*c;

	if (!g_log.n)
		return ;
	c = &g_log.calls[g_log.n - 1];
	if (len > 63)
		len = 63;
	if (len < 0)
		len = 0;
	memcpy(c->text, text, (size_t)len);
	c->text[len] = '\0';
}

int	fake_log_count(void)
{
	return (g_log.n);
}
