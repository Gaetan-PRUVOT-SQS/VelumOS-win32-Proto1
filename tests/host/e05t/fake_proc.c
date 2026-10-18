#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"

t_ft	g_ft;

static t_process	g_proc;
static t_thread		g_thread;
static int			g_aspace;

void	ft_reset(uint64_t limit, uint64_t pages)
{
	memset(&g_ft, 0, sizeof(g_ft));
	memset(&g_proc, 0, sizeof(g_proc));
	memset(&g_thread, 0, sizeof(g_thread));
	g_thread.tid = 42;
	g_proc.mem_limit = limit;
	g_proc.aspace = (t_aspace *)&g_aspace;
	g_ft.pages = pages;
}

int64_t	ft_create(uint64_t size)
{
	t_sysargs	a;

	memset(&a, 0, sizeof(a));
	a.a[0] = 0x400000;
	a.a[2] = size;
	return (sys_thread_create(&a));
}

t_process	*proc_current(void)
{
	return (&g_proc);
}

uint64_t	vmm_pages_used(const t_aspace *as)
{
	(void)as;
	return (g_ft.pages);
}

int	syscall_prio_check(uint64_t prio)
{
	(void)prio;
	return (0);
}

int	proc_tstack_alloc(t_process *p, uint64_t size, uint64_t arg,
		t_pthread *st)
{
	(void)p;
	(void)arg;
	g_ft.allocs++;
	if (size != 0 && (size < TSTACK_MIN || size > TSTACK_MAX))
		return (E_INVAL);
	g_ft.last_size = size;
	st->stack_va = 0x10000000;
	st->stack_len = 0x41000;
	return (0);
}

t_thread	*proc_thread_add(t_process *p, const t_threadreq *rq,
				const t_pthread *st)
{
	(void)p;
	(void)rq;
	(void)st;
	if (g_ft.fail_add)
		return (NULL);
	return (&g_thread);
}

void	thread_unref(t_thread *t)
{
	(void)t;
}

int	vmm_stack_unmap(t_aspace *as, uintptr_t va, size_t len)
{
	(void)as;
	(void)va;
	(void)len;
	g_ft.unmaps++;
	return (0);
}

void	klog_warn(const char *fmt, ...)
{
	(void)fmt;
}

void	proc_exit_current(int code)
{
	(void)code;
	abort();
}

void	proc_thread_leave(t_process *p, t_thread *t, int code)
{
	(void)p;
	(void)t;
	(void)code;
}

t_thread	*sched_current(void)
{
	return (&g_thread);
}

void	thread_exit(int code)
{
	(void)code;
	abort();
}
