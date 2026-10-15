#include "fake.h"
#include "fat.h"

uint32_t	fake_fsi(const t_fakeblk *b, uint32_t off)
{
	t_fakegeo	g;

	fakefat_geo(b, &g);
	return (le32(b->img + g.bps + off));
}

void	fake_fsi_set(t_fakeblk *b, uint32_t off, uint32_t v)
{
	t_fakegeo	g;

	fakefat_geo(b, &g);
	put32(b->img + g.bps + off, v);
}

uint32_t	fake_fat_at(const t_fakeblk *b, uint32_t c)
{
	t_fakegeo	g;

	fakefat_geo(b, &g);
	return (le32(b->img + (uint64_t)g.rsvd * g.bps + (uint64_t)c * 4)
		& FAT_MASK);
}

uint32_t	fake_free_real(const t_fakeblk *b)
{
	t_fakegeo	g;
	uint32_t	c;
	uint32_t	n;

	fakefat_geo(b, &g);
	n = 0;
	c = 2;
	while (c <= g.nclus + 1)
	{
		if (fake_fat_at(b, c) == 0)
			n++;
		c++;
	}
	return (n);
}

int64_t	fake_free_gap(const t_fakeblk *b)
{
	return ((int64_t)fake_fsi(b, FAKE_FSI_FREE)
		- (int64_t)fake_free_real(b));
}
