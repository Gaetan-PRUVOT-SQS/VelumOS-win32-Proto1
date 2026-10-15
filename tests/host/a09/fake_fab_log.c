#include "fake_fab.h"

uint32_t	fab_writes_at(uint16_t lo, uint16_t hi)
{
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < g_fab.nlog)
	{
		n += g_fab.log[i].off >= lo && g_fab.log[i].off < hi;
		i++;
	}
	return (n);
}
