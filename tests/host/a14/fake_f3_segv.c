#include <setjmp.h>
#include <signal.h>
#include "fake_f3.h"

static sigjmp_buf	g_f3_jmp;

static void	f3_on_segv(int sig)
{
	(void)sig;
	siglongjmp(g_f3_jmp, 1);
}

int	f3_survives(void (*fn)(void *), void *arg)
{
	struct sigaction	sa;
	struct sigaction	old;
	int					ok;

	sa.sa_handler = f3_on_segv;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGSEGV, &sa, &old);
	ok = 0;
	if (sigsetjmp(g_f3_jmp, 1) == 0)
	{
		fn(arg);
		ok = 1;
	}
	sigaction(SIGSEGV, &old, NULL);
	return (ok);
}
