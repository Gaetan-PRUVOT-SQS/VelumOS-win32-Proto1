#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuzz.h"
#include "velum/heap.h"

typedef struct s_fuzzheap
{
	int64_t	left;
	int64_t	live;
	int		armed;
}	t_fuzzheap;

static t_fuzzheap	g_heap;

void	fuzz_violation(const char *what, const char *file, int line)
{
	fprintf(stderr, "FUZZ-VIOLATION %s:%d : %s\n", file, line, what);
	abort();
}

void	fuzz_heap_fail_after(int64_t n)
{
	g_heap.armed = (n >= 0);
	g_heap.left = n;
}

int64_t	fuzz_heap_live(void)
{
	return (g_heap.live);
}

void	*kmalloc_tag(size_t size, t_heap_tag tag)
{
	uint64_t	*p;

	(void)tag;
	if (g_heap.armed && g_heap.left == 0)
		return (NULL);
	if (g_heap.armed)
		g_heap.left--;
	p = malloc(size + 16);
	if (p == NULL)
		return (NULL);
	p[1] = size;
	memset(p + 2, 0xA5, size);
	g_heap.live++;
	return (p + 2);
}

void	kfree(void *ptr)
{
	if (ptr == NULL)
		return ;
	g_heap.live--;
	free((uint64_t *)ptr - 2);
}
