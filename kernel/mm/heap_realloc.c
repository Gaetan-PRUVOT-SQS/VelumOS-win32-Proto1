#include "heap_int.h"
#include "velum/libk.h"

void	*heap_resize(void *p, const t_block *b, size_t size, const void *at)
{
	t_heap_req	rq;
	void		*np;

	if (heap_block_fits(b, size))
	{
		heap_block_restamp(p, b, size);
		return (p);
	}
	heap_req_set(&rq, size, b->tag, at);
	np = heap_alloc(&rq);
	if (!np && size < b->cap)
	{
		heap_block_restamp(p, b, size);
		return (p);
	}
	if (!np)
		return (NULL);
	memcpy(np, p, min_u64(b->used, size));
	heap_free(p, at);
	return (np);
}

void	*krealloc(void *ptr, size_t size)
{
	t_heap_req	rq;
	t_block		b;
	const void	*at;

	at = __builtin_return_address(0);
	if (!ptr)
	{
		heap_req_set(&rq, size, HEAP_GENERIC, at);
		return (heap_alloc(&rq));
	}
	if (size == 0)
	{
		heap_free(ptr, at);
		return (NULL);
	}
	heap_block_info(ptr, at, &b);
	return (heap_resize(ptr, &b, size, at));
}

void	*kcalloc(size_t n, size_t size)
{
	t_heap_req	rq;
	size_t		total;
	void		*p;

	if (__builtin_mul_overflow(n, size, &total))
		return (heap_fail());
	heap_req_set(&rq, total, HEAP_GENERIC, __builtin_return_address(0));
	p = heap_alloc(&rq);
	if (p)
		memset(p, 0, total);
	return (p);
}
