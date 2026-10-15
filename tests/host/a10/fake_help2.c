#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "th.h"

uint64_t	th_rand(uint64_t *state)
{
	uint64_t	x;

	x = *state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	*state = x;
	return (x);
}

uint64_t	th_seed(const char *suite)
{
	const char	*env;
	uint64_t	seed;

	seed = 0x9e3779b97f4a7c15ull;
	env = getenv("A10_SEED");
	if (env)
		seed = strtoull(env, NULL, 0);
	printf("%s : graine %#llx\n", suite, (unsigned long long)seed);
	return (seed);
}

t_inpevent	th_event(uint32_t type, uint32_t code)
{
	t_inpevent	e;

	memset(&e, 0, sizeof(e));
	e.type = type;
	e.code = code;
	return (e);
}

int	th_ev_is(const t_inpevent *e, uint32_t type, uint32_t code)
{
	return (e->type == type && e->code == code);
}

int32_t	th_accel_step(t_accel *a, int32_t dx, int32_t dy, int32_t *oy)
{
	int32_t	x;
	int32_t	y;

	x = dx;
	y = dy;
	accel_apply(a, &x, &y);
	*oy = y;
	return (x);
}
