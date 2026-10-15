#include "velum/crypto.h"

int	ct_equal(const void *a, const void *b, size_t n)
{
	const uint8_t	*pa;
	const uint8_t	*pb;
	uint32_t		diff;
	size_t			i;

	if (!n)
		return (1);
	if (!a || !b)
		return (0);
	pa = a;
	pb = b;
	diff = 0;
	i = 0;
	while (i < n)
	{
		diff |= (uint32_t)(pa[i] ^ pb[i]);
		i++;
	}
	return ((int)(((diff - 1) >> 31) & 1));
}

void	secure_zero(void *p, size_t n)
{
	volatile uint8_t	*v;

	if (!p)
		return ;
	v = p;
	while (n)
	{
		*v = 0;
		v++;
		n--;
	}
}
