#include "stdlib.h"

int	abs(int n)
{
	if (n < 0)
		return ((int)(0u - (unsigned int)n));
	return (n);
}

long	labs(long n)
{
	if (n < 0)
		return ((long)(0ul - (unsigned long)n));
	return (n);
}

long long	llabs(long long n)
{
	if (n < 0)
		return ((long long)(0ull - (unsigned long long)n));
	return (n);
}

int	atoi(const char *s)
{
	return ((int)strtol(s, NULL, 10));
}

long	atol(const char *s)
{
	return (strtol(s, NULL, 10));
}
