#include "fake.h"

t_fproc	g_fproc;

t_process	*proc_current(void)
{
	if (!g_fproc.present)
		return (NULL);
	return (&g_fproc.proc);
}

void	fproc_set(uint32_t flags)
{
	g_fproc.proc.pid = 42;
	g_fproc.proc.flags = flags;
	g_fproc.present = 1;
}

void	fproc_clear(void)
{
	g_fproc.present = 0;
}
