#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"

static t_fakeblk	*g_devs[FAKE_DEVS];

t_fakeblk	*fakeblk_new(uint64_t nsec, uint32_t ssz)
{
	t_fakeblk	*b;
	uint32_t	i;

	b = calloc(1, sizeof(*b));
	if (b == NULL)
		abort();
	b->img = calloc(nsec, ssz);
	if (b->img == NULL)
		abort();
	b->dev.sector_size = ssz;
	b->dev.nsectors = nsec;
	b->dev.priv = b;
	b->wfail = -1;
	b->rfail = -1;
	b->watch = -1;
	i = 0;
	while (i < FAKE_DEVS && g_devs[i])
		i++;
	if (i < FAKE_DEVS)
		g_devs[i] = b;
	snprintf(b->dev.name, BLK_NAME_MAX, "fk%u", i);
	return (b);
}

void	fakeblk_free(t_fakeblk *b)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_DEVS)
	{
		if (g_devs[i] == b)
			g_devs[i] = NULL;
		i++;
	}
	free(b->img);
	free(b);
}

uint32_t	blk_count(void)
{
	return (FAKE_DEVS);
}

t_blkdev	*blk_at(uint32_t i)
{
	if (i >= FAKE_DEVS || g_devs[i] == NULL)
		return (NULL);
	return (&g_devs[i]->dev);
}
