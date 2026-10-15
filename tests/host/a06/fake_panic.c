#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "fake_sched.h"
#include "velum/panic.h"

static t_fpanic	g_fp;

_Noreturn void	panic(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_fp.msg, sizeof(g_fp.msg), fmt, ap);
	va_end(ap);
	g_fp.count++;
	if (g_fp.armed && g_fp.owner == (void *)fake_self())
	{
		g_fp.armed = 0;
		longjmp(g_fp.jb, 1);
	}
	fprintf(stderr, "PANIC (hôte) : %s\n", g_fp.msg);
	abort();
}

void	kassert_check(int cond, const char *msg)
{
	if (!cond)
		panic("assertion: %s", msg);
}

jmp_buf	*fake_panic_arm(void)
{
	g_fp.owner = fake_self();
	g_fp.msg[0] = '\0';
	g_fp.armed = 1;
	return (&g_fp.jb);
}

void	fake_panic_disarm(void)
{
	g_fp.armed = 0;
}

const char	*fake_panic_msg(void)
{
	return (g_fp.msg);
}
