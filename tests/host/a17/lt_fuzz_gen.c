#include "lt_fuzz.h"

#define NEXT 10

static const int32_t	g_ext[NEXT] = {INT32_MIN, INT32_MIN + 1, -16777217,
	-16777216, 16777216, 16777217, INT32_MAX - 1, INT32_MAX, 0, -1};

static int32_t	ltf_pick(uint64_t *seed, int32_t lo, uint32_t span)
{
	if (ltf_rnd(seed, 6) == 0)
		return (g_ext[ltf_rnd(seed, NEXT)]);
	return (lo + (int32_t)ltf_rnd(seed, span));
}

void	ltf_case(uint64_t *seed, int32_t *c)
{
	c[0] = ltf_pick(seed, -30, 160);
	c[1] = ltf_pick(seed, -30, 100);
	c[2] = ltf_pick(seed, -3, 130);
	c[3] = ltf_pick(seed, -3, 90);
	c[4] = ltf_pick(seed, -20, 140);
	c[5] = ltf_pick(seed, -20, 90);
	c[6] = ltf_pick(seed, -3, 160);
	c[7] = ltf_pick(seed, -3, 100);
	if (ltf_rnd(seed, 2) == 0)
	{
		c[4] = 0;
		c[5] = 0;
		c[6] = 100;
		c[7] = 60;
	}
	c[8] = (int32_t)ltf_rnd(seed, LUNA_GLASS_KINDS + 2) - 1;
	c[9] = (int32_t)ltf_rnd(seed, 64);
}
