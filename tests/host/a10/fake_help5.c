#include <stdlib.h>
#include "th.h"

int	th_popcount(uint32_t v)
{
	int	n;

	n = 0;
	while (v)
	{
		n += (int)(v & 1);
		v >>= 1;
	}
	return (n);
}

uint64_t	th_pushes(void)
{
	const char	*env;

	env = getenv("A10_PUSHES");
	if (env)
		return (strtoull(env, NULL, 0));
	return (1000);
}
