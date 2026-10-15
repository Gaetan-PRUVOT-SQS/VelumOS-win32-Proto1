#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "velum/panic.h"
#include "a02_fake.h"

static jmp_buf		g_jump;
static const char	*g_msg;
static int			g_armed;
static int			g_count;

void	kassert_check(int cond, const char *msg)
{
	if (cond)
		return ;
	g_msg = msg;
	g_count++;
	if (!g_armed)
	{
		fprintf(stderr, "kassert hors garde : %s\n", msg);
		abort();
	}
	longjmp(g_jump, 1);
}

int	fake_catch(void (*fn)(void *), void *arg)
{
	g_armed = 1;
	if (setjmp(g_jump) == 0)
	{
		fn(arg);
		g_armed = 0;
		return (0);
	}
	g_armed = 0;
	return (1);
}

const char	*fake_assert_msg(void)
{
	return (g_msg);
}

int	fake_assert_count(void)
{
	return (g_count);
}
