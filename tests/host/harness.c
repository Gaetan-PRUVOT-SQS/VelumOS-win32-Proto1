#include <stdio.h>
#include "harness.h"

t_hstate	g_h = {"?", "?", 0, 0};

void	h_begin(const char *suite)
{
	g_h.suite = suite;
	g_h.fail = 0;
	g_h.checks = 0;
}

void	h_run(const char *name, void (*fn)(void))
{
	g_h.name = name;
	fn();
}

void	h_true(int cond, const char *what)
{
	g_h.checks++;
	if (cond)
		return ;
	g_h.fail++;
	fprintf(stderr, "ECHEC %s/%s : %s\n", g_h.suite, g_h.name, what);
}

int	h_end(void)
{
	printf("%s : %d verifications, %d echec(s)\n", g_h.suite, g_h.checks,
		g_h.fail);
	return (g_h.fail != 0);
}
