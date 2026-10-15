#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "a04_fake.h"
#include "velum/panic.h"

static jmp_buf	g_jump;
static int		g_armed;
static char		g_msg[320];

_Noreturn void	panic(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_msg, sizeof(g_msg), fmt, ap);
	va_end(ap);
	if (g_armed)
	{
		g_armed = 0;
		longjmp(g_jump, 1);
	}
	fprintf(stderr, "PANIC inattendue : %s\n", g_msg);
	abort();
}

void	kassert_check(int cond, const char *msg)
{
	if (!cond)
		panic("assertion: %s", msg);
}

int	fake_catch(void (*fn)(void *), void *arg)
{
	g_msg[0] = '\0';
	if (setjmp(g_jump))
		return (1);
	g_armed = 1;
	fn(arg);
	g_armed = 0;
	return (0);
}

const char	*fake_panic_msg(void)
{
	return (g_msg);
}
