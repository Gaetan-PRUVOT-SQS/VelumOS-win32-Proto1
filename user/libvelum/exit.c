#include "exit_int.h"
#include "velum/vmisc.h"
#include "velum/vproc.h"

static t_atexit	g_atexit;

int	atexit(void (*fn)(void))
{
	int	rc;

	if (!fn)
		return (-1);
	rc = -1;
	v_spin_lock(&g_atexit.lock);
	if (g_atexit.count < ATEXIT_MAX)
	{
		g_atexit.fns[g_atexit.count] = fn;
		g_atexit.count++;
		rc = 0;
	}
	v_spin_unlock(&g_atexit.lock);
	return (rc);
}

void	exit_run_handlers(void)
{
	t_atexitfn	fn;

	fn = NULL;
	while (1)
	{
		v_spin_lock(&g_atexit.lock);
		fn = NULL;
		if (g_atexit.count)
		{
			g_atexit.count--;
			fn = g_atexit.fns[g_atexit.count];
		}
		v_spin_unlock(&g_atexit.lock);
		if (!fn)
			return ;
		fn();
	}
}

_Noreturn void	exit(int status)
{
	exit_run_handlers();
	v_exit(status);
}

_Noreturn void	abort(void)
{
	v_log(V_LOG_ERR, "abort()");
	v_exit(EXIT_ABORT_CODE);
}
