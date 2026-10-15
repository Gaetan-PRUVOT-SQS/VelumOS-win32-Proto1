#include "fake_alloc.h"
#include "fake_f3.h"

void	f3_pattern(int *v, size_t n, int mode, uint64_t seed)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (mode == 0)
			v[i] = (int)i;
		else if (mode == 1)
			v[i] = (int)(n - i);
		else if (mode == 2)
			v[i] = 7;
		else if (mode == 3)
			v[i] = (int)(i * 37 % 7);
		else if (mode == 4)
			v[i] = (int)i + (int)(n - 2 * i) * (i >= n / 2);
		else if (mode == 5)
			v[i] = (int)(i % 10);
		else
			v[i] = (int)(fa_rng(&seed) % 2000001) - 1000000;
		i++;
	}
}

int	f3_same_ints(const int *a, const int *b, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (a[i] != b[i])
			return (0);
		i++;
	}
	return (1);
}
