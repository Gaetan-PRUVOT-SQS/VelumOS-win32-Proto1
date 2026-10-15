#include "heap_int.h"
#include "velum/libk.h"

void	*kmalloc(size_t size)
{
	t_heap_req	rq;

	heap_req_set(&rq, size, HEAP_GENERIC, __builtin_return_address(0));
	return (heap_alloc(&rq));
}

void	*kmalloc_tag(size_t size, t_heap_tag tag)
{
	t_heap_req	rq;

	heap_req_set(&rq, size, tag, __builtin_return_address(0));
	return (heap_alloc(&rq));
}

void	*kmalloc_aligned(size_t size, size_t align)
{
	t_heap_req	rq;

	heap_req_set(&rq, size, HEAP_GENERIC, __builtin_return_address(0));
	rq.align = align;
	return (heap_alloc(&rq));
}

void	*kzalloc(size_t size)
{
	t_heap_req	rq;
	void		*p;

	heap_req_set(&rq, size, HEAP_GENERIC, __builtin_return_address(0));
	p = heap_alloc(&rq);
	if (p)
		memset(p, 0, size);
	return (p);
}

void	kfree(void *ptr)
{
	heap_free(ptr, __builtin_return_address(0));
}
