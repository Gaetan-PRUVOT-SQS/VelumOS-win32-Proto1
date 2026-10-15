#include "alloc_int.h"
#include "velum/vmem.h"

static t_ablock	*alloc_reject(int code)
{
	alloc_fault(code);
	return (NULL);
}

t_ablock	*alloc_check(const void *p)
{
	t_ablock	*b;
	uint32_t	state;

	if ((uintptr_t)p & (ALLOC_ALIGN - 1))
		return (alloc_reject(ALLOC_FAULT_BAD_POINTER));
	if (alloc_large_recent(p))
		return (alloc_reject(ALLOC_FAULT_DOUBLE_FREE));
	b = (t_ablock *)p - 1;
	state = alloc_state_of(b);
	if (state == ALLOC_TAG_FREE)
		return (alloc_reject(ALLOC_FAULT_DOUBLE_FREE));
	if (state != ALLOC_TAG_USED)
		return (alloc_reject(ALLOC_FAULT_CORRUPT));
	return (b);
}

size_t	alloc_capacity(const t_ablock *b)
{
	if (b->cls == ALLOC_CLS_LARGE)
		return ((size_t)b->pages * PAGE_SIZE - ALLOC_HDR);
	return (alloc_class_size(b->cls));
}

size_t	alloc_probe(const void *p)
{
	t_ablock	*b;
	size_t		cap;

	cap = 0;
	v_spin_lock(&g_alloc.lock);
	b = alloc_check(p);
	if (b)
		cap = alloc_capacity(b);
	v_spin_unlock(&g_alloc.lock);
	return (cap);
}
