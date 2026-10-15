#include <stddef.h>

int	memcmp(const void *a, const void *b, size_t n)
{
	const unsigned char	*pa;
	const unsigned char	*pb;

	pa = a;
	pb = b;
	while (n--)
	{
		if (*pa != *pb)
			return ((int)*pa - (int)*pb);
		pa++;
		pb++;
	}
	return (0);
}

void	*memchr(const void *s, int c, size_t n)
{
	const unsigned char	*p;

	p = s;
	while (n--)
	{
		if (*p == (unsigned char)c)
			return ((void *)p);
		p++;
	}
	return (NULL);
}
