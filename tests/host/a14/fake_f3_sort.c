#include "fake_f3.h"

static uint64_t	g_f3_calls;

int	f3_cmp_int(const void *a, const void *b)
{
	int	x;
	int	y;

	x = *(const int *)a;
	y = *(const int *)b;
	return ((x > y) - (x < y));
}

int	f3_cmp_int_desc(const void *a, const void *b)
{
	return (f3_cmp_int(b, a));
}

int	f3_cmp_counting(const void *a, const void *b)
{
	g_f3_calls++;
	return (f3_cmp_int(a, b));
}

uint64_t	f3_cmp_calls(void)
{
	return (g_f3_calls);
}

void	f3_cmp_reset(void)
{
	g_f3_calls = 0;
}
