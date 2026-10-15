#include <stdlib.h>
#include "a02_fake.h"

static uint64_t	g_state = FAKE_DEFAULT_SEED;
static uint64_t	g_seed = FAKE_DEFAULT_SEED;

void	fake_seed(uint64_t seed)
{
	const char	*env;

	env = getenv("A02_SEED");
	if (env)
		seed = strtoull(env, NULL, 0);
	if (!seed)
		seed = 1;
	g_seed = seed;
	g_state = seed;
}

uint64_t	fake_seed_value(void)
{
	return (g_seed);
}

uint64_t	fake_rand(void)
{
	g_state ^= g_state >> 12;
	g_state ^= g_state << 25;
	g_state ^= g_state >> 27;
	return (g_state * 0x2545f4914f6cdd1dull);
}

uint64_t	fake_below(uint64_t bound)
{
	if (!bound)
		return (0);
	return (fake_rand() % bound);
}
