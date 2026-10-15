#include "sort_int.h"
#include "stdlib.h"

void	*vbsearch_impl(const t_bsearch *a)
{
	size_t				lo;
	size_t				hi;
	size_t				mid;
	int					c;
	const unsigned char	*p;

	lo = 0;
	hi = a->nmemb;
	if (!a->cmp || !a->size || !a->base)
		return (NULL);
	while (lo < hi)
	{
		mid = lo + (hi - lo) / 2;
		p = (const unsigned char *)a->base + mid * a->size;
		c = a->cmp(a->key, p);
		if (c == 0)
			return ((void *)p);
		if (c < 0)
			hi = mid;
		else
			lo = mid + 1;
	}
	return (NULL);
}
