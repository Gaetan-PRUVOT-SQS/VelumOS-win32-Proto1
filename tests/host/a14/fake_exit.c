#include "fake_int.h"
#include "fake_sys.h"
#include "velum/err.h"

t_fsys	g_fsys;

int64_t	fake_exit(const uint64_t *a)
{
	g_fsys.exit_code = (int)a[0];
	if (g_fsys.exit_armed)
	{
		g_fsys.exit_armed = 0;
		longjmp(g_fsys.exit_jmp, 1);
	}
	return (0);
}

int64_t	fake_exit_catch(void (*fn)(void))
{
	g_fsys.exit_armed = 1;
	if (setjmp(g_fsys.exit_jmp) == 0)
	{
		fn();
		g_fsys.exit_armed = 0;
		return (FAKE_NO_EXIT);
	}
	g_fsys.exit_armed = 0;
	return (g_fsys.exit_code);
}

int64_t	fake_close(const uint64_t *a)
{
	if (fake_event_close(a[0]) == 0)
		return (0);
	return (fake_thread_close(a[0]));
}
