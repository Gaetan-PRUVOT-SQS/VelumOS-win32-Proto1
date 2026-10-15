#include "a09_test.h"
#include "velum/random.h"
#include "velum/util.h"

static const size_t	g_sizes[6] = {1, 31, 32, 33, 100, 4096};

void	fill_random(uint8_t *buf, size_t n)
{
	size_t	off;
	size_t	take;
	size_t	k;

	off = 0;
	k = 0;
	while (off < n)
	{
		take = min_u64(g_sizes[k % 6], n - off);
		krandom(buf + off, take);
		off += take;
		k++;
	}
}
