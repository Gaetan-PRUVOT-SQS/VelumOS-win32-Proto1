#include "fake_f3.h"
#include "string.h"

int	f3_cmp_b1(const void *a, const void *b)
{
	return (memcmp(a, b, 1));
}

int	f3_cmp_b3(const void *a, const void *b)
{
	return (memcmp(a, b, 3));
}

int	f3_cmp_b8(const void *a, const void *b)
{
	return (memcmp(a, b, 8));
}

int	f3_cmp_b24(const void *a, const void *b)
{
	return (memcmp(a, b, 24));
}
