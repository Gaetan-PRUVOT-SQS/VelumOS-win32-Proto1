#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "fat.h"

uint32_t	fsck_fat(const t_fsck *k, uint32_t c)
{
	return (le32(k->b->img + (uint64_t)k->g.rsvd * k->g.bps
			+ (uint64_t)c * 4) & 0x0FFFFFFF);
}

uint8_t	*fsck_clus(const t_fsck *k, uint32_t c)
{
	uint64_t	sec;

	sec = k->g.rsvd + (uint64_t)k->g.nfats * k->g.fatsz
		+ (uint64_t)(c - 2) * k->g.spc;
	return (k->b->img + sec * k->g.bps);
}

static void	check_mirror(t_fsck *k)
{
	uint64_t	len;
	uint8_t		*f0;

	len = ((uint64_t)k->g.nclus + 2) * 4;
	f0 = k->b->img + (uint64_t)k->g.rsvd * k->g.bps;
	if (memcmp(f0, f0 + (uint64_t)k->g.fatsz * k->g.bps, len) != 0)
	{
		fprintf(stderr, "fsck: FAT miroir différente\n");
		k->errs++;
	}
}

static void	check_free(t_fsck *k)
{
	uint32_t	c;
	uint32_t	free_n;
	uint32_t	info;

	free_n = 0;
	c = 2;
	while (c <= k->g.nclus + 1)
	{
		if (fsck_fat(k, c) == 0)
			free_n++;
		else if (!k->used[c] && fsck_fat(k, c) != 0x0FFFFFF7)
		{
			fprintf(stderr, "fsck: cluster %u perdu\n", c);
			k->errs++;
		}
		c++;
	}
	info = le32(k->b->img + k->g.bps + 488);
	if (info != 0xFFFFFFFF && info != free_n)
	{
		fprintf(stderr, "fsck: FSInfo %u libres, réel %u\n", info, free_n);
		k->errs++;
	}
}

int	fake_fsck(const t_fakeblk *b)
{
	t_fsck	k;

	memset(&k, 0, sizeof(k));
	k.b = b;
	fakefat_geo(b, &k.g);
	k.used = calloc((size_t)k.g.nclus + 2, 1);
	check_mirror(&k);
	k.stack[k.top++] = 2;
	while (k.top && k.errs < 20)
		fsck_dir(&k, k.stack[--k.top]);
	check_free(&k);
	free(k.used);
	return (k.errs);
}
