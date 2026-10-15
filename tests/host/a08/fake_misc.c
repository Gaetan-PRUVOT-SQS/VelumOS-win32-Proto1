#include <stdio.h>
#include <stdlib.h>
#include "fake.h"

void	kassert_check(int cond, const char *msg)
{
	if (cond)
		return ;
	fprintf(stderr, "kassert : %s\n", msg);
	abort();
}

void	klog_warn(const char *fmt, ...)
{
	(void)fmt;
}

t_process	*fk_proc_new(uint32_t flags)
{
	t_process	*p;

	p = calloc(1, sizeof(t_process));
	if (!p)
		abort();
	p->handles = handle_table_create();
	p->aspace = calloc(1, 64);
	if (!p->handles || !p->aspace)
		abort();
	p->flags = flags;
	p->mem_limit = 0x4000000;
	return (p);
}

void	fk_proc_free(t_process *p)
{
	object_process_cleanup(p);
	handle_table_destroy(p->handles);
	free(p->aspace);
	free(p);
}
