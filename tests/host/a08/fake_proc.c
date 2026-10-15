#include "fake.h"

t_process	*proc_current(void)
{
	return (g_fk.cur);
}

void	proc_ref(t_process *p)
{
	(void)p;
	g_fk.proc_refs++;
}

void	proc_unref(t_process *p)
{
	(void)p;
	g_fk.proc_refs--;
}

void	thread_ref(t_thread *t)
{
	(void)t;
	g_fk.thread_refs++;
}

void	thread_unref(t_thread *t)
{
	(void)t;
	g_fk.thread_refs--;
}
