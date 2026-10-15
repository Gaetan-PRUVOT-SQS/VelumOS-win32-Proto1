#include <stdint.h>
#include "sort_int.h"
#include "stdlib.h"

static void	sort_swap(unsigned char *a, unsigned char *b, size_t size)
{
	unsigned char	tmp;

	while (size--)
	{
		tmp = *a;
		*a = *b;
		*b = tmp;
		a++;
		b++;
	}
}

static unsigned char	*sort_at(const t_sortctx *c, size_t i)
{
	return (c->base + i * c->size);
}

static void	sort_sift(const t_sortctx *c, size_t root, size_t n)
{
	size_t	child;

	while (root * 2 + 1 < n)
	{
		child = root * 2 + 1;
		if (child + 1 < n
			&& c->cmp(sort_at(c, child), sort_at(c, child + 1)) < 0)
			child++;
		if (c->cmp(sort_at(c, root), sort_at(c, child)) >= 0)
			return ;
		sort_swap(sort_at(c, root), sort_at(c, child), c->size);
		root = child;
	}
}

void	qsort(void *base, size_t nmemb, size_t size,
		int (*cmp)(const void *, const void *))
{
	t_sortctx	c;
	size_t		i;
	size_t		total;

	if (!base || !cmp || nmemb < 2 || !size || nmemb > SIZE_MAX / 2
		|| __builtin_mul_overflow(nmemb, size, &total))
		return ;
	c.base = base;
	c.size = size;
	c.cmp = cmp;
	i = nmemb / 2;
	while (i--)
		sort_sift(&c, i, nmemb);
	i = nmemb;
	while (--i > 0)
	{
		sort_swap(sort_at(&c, 0), sort_at(&c, i), size);
		sort_sift(&c, 0, i);
	}
}
