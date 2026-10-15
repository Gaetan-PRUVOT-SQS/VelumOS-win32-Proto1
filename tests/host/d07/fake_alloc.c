#undef malloc
#include <stdlib.h>
#include "d07.h"

static int	g_d07_left = -1;

void	d07_fail_after(int n)
{
	g_d07_left = n;
}

void	*d07_malloc(size_t n)
{
	if (g_d07_left == 0)
		return (NULL);
	if (g_d07_left > 0)
		g_d07_left--;
	return (malloc(n));
}
