#include "fake.h"
#include "fat.h"

int	fakefat_mount(t_fakeblk *b, const char *at)
{
	return (vfs_mount(at, "fat32", &b->dev, 0));
}

void	fakefat_bad_from(t_fakeblk *b, uint32_t first)
{
	t_fakegeo	g;
	uint32_t	c;

	fakefat_geo(b, &g);
	c = first;
	while (c <= g.nclus + 1)
		fakefat_set(b, c++, 0x0FFFFFF7);
	put32(b->img + g.bps + 488, 0xFFFFFFFF);
}

uint8_t	*fake_rootent(t_fakeblk *b, uint32_t idx)
{
	t_fakegeo	g;

	fakefat_geo(b, &g);
	return (b->img + ((uint64_t)g.rsvd + (uint64_t)g.nfats * g.fatsz)
		* g.bps + (uint64_t)idx * 32);
}
