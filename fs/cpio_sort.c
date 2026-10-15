#include "cpio.h"

static uint32_t	sort_key(char c)
{
	if (c == '/')
		return (1);
	return ((uint8_t)c);
}

int	cpio_cmp(const char *a, uint32_t an, const char *b, uint32_t bn)
{
	uint32_t	i;

	i = 0;
	while (i < an && i < bn)
	{
		if (sort_key(a[i]) != sort_key(b[i]))
		{
			if (sort_key(a[i]) < sort_key(b[i]))
				return (-1);
			return (1);
		}
		i++;
	}
	if (an == bn)
		return (0);
	if (an < bn)
		return (-1);
	return (1);
}

static int	ent_less(const t_cpent *a, const t_cpent *b)
{
	int	c;

	c = cpio_cmp(a->name, a->nlen, b->name, b->nlen);
	if (c)
		return (c < 0);
	return (a->order > b->order);
}

static void	sift(t_cpent *v, uint32_t root, uint32_t n)
{
	uint32_t	child;
	t_cpent		tmp;

	while (2 * root + 1 < n)
	{
		child = 2 * root + 1;
		if (child + 1 < n && ent_less(&v[child], &v[child + 1]))
			child++;
		if (!ent_less(&v[root], &v[child]))
			return ;
		tmp = v[root];
		v[root] = v[child];
		v[child] = tmp;
		root = child;
	}
}

void	cpio_sort(t_cpent *v, uint32_t n)
{
	uint32_t	i;
	t_cpent		tmp;

	i = n / 2;
	while (i > 0)
	{
		i--;
		sift(v, i, n);
	}
	i = n;
	while (i > 1)
	{
		i--;
		tmp = v[0];
		v[0] = v[i];
		v[i] = tmp;
		sift(v, 0, i);
	}
}
