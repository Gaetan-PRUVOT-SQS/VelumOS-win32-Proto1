#include "fake.h"
#include "velum/heap.h"
#include "velum/proc.h"

int	proc_count(void)
{
	return ((int)g_fake.nprocs);
}

void	heap_fail_after(int64_t n)
{
	g_fake.alloc_fail = (n >= 0);
}
