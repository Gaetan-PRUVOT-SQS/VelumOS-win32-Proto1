#include "alloc_int.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"

static void	*alloc_get(size_t size)
{
	int		cls;
	void	*p;

	cls = alloc_class_of(size);
	if (cls >= 0)
		p = alloc_small_get((uint32_t)cls);
	else
		p = alloc_large_get(size);
	if (p)
		g_alloc.stats.alloc_calls++;
	return (p);
}

void	*malloc(size_t size)
{
	void	*p;

	if (size > ALLOC_MAX)
	{
		errno = ENOMEM;
		return (NULL);
	}
	v_spin_lock(&g_alloc.lock);
	p = alloc_get(size);
	v_spin_unlock(&g_alloc.lock);
	if (!p)
		errno = ENOMEM;
	return (p);
}

void	free(void *p)
{
	t_ablock	*b;

	if (!p)
		return ;
	v_spin_lock(&g_alloc.lock);
	b = alloc_check(p);
	if (b)
	{
		if (b->cls == ALLOC_CLS_LARGE)
			alloc_large_put(b);
		else
			alloc_small_put(b);
		g_alloc.stats.free_calls++;
	}
	v_spin_unlock(&g_alloc.lock);
}

void	*reallocarray(void *p, size_t nmemb, size_t size)
{
	size_t	total;

	if (__builtin_mul_overflow(nmemb, size, &total))
	{
		errno = ENOMEM;
		return (NULL);
	}
	return (realloc(p, total));
}

void	*calloc(size_t nmemb, size_t size)
{
	size_t	total;
	void	*p;

	if (__builtin_mul_overflow(nmemb, size, &total))
	{
		errno = ENOMEM;
		return (NULL);
	}
	p = malloc(total);
	if (p && total <= ALLOC_SMALL_MAX)
		memset(p, 0, total);
	return (p);
}
