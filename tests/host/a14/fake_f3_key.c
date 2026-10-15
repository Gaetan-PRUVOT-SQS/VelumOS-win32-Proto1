#include "fake_f3.h"

static const void	*g_f3_key;
static uint64_t		g_f3_violations;

void	f3_key_expect(const void *key)
{
	g_f3_key = key;
	g_f3_violations = 0;
}

int	f3_cmp_keyed(const void *a, const void *b)
{
	if (a != g_f3_key || b == g_f3_key)
		g_f3_violations++;
	return (f3_cmp_int(a, b));
}

uint64_t	f3_key_violations(void)
{
	return (g_f3_violations);
}
