#include <stdio.h>
#include <string.h>
#include "fake.h"
#include "fat.h"

static uint32_t	mark_chain(t_fsck *k, uint32_t c)
{
	uint32_t	n;

	n = 0;
	while (c >= 2 && c <= k->g.nclus + 1)
	{
		if (k->used[c])
		{
			fprintf(stderr, "fsck: cluster %u croisé ou boucle\n", c);
			k->errs++;
			return (n);
		}
		k->used[c] = 1;
		n++;
		c = fsck_fat(k, c);
	}
	if (c < 0x0FFFFFF8)
	{
		fprintf(stderr, "fsck: chaîne cassée (%u)\n", c);
		k->errs++;
	}
	return (n);
}

static void	check_entry(t_fsck *k, const uint8_t *e)
{
	uint32_t	first;
	uint32_t	n;
	uint64_t	size;
	uint64_t	clsz;

	first = (le16(e + 20) << 16) | le16(e + 26);
	size = le32(e + 28);
	clsz = (uint64_t)k->g.bps * k->g.spc;
	n = 0;
	if (first)
		n = mark_chain(k, first);
	if ((e[11] & 0x10) && first && k->top < 1024)
		k->stack[k->top++] = first;
	else if (!(e[11] & 0x10) && (n * clsz < size
			|| (n && (n - 1) * clsz >= size)))
	{
		fprintf(stderr, "fsck: taille %llu pour %u clusters (%.11s)\n",
			(unsigned long long)size, n, (const char *)e);
		k->errs++;
	}
}

static int	dir_entry(t_fsck *k, const uint8_t *e)
{
	if (e[0] == 0)
		return (0);
	if (e[0] == 0xE5 || (e[11] & 0x3F) == 0x0F || (e[11] & 0x08)
		|| e[0] == '.')
		return (1);
	check_entry(k, e);
	return (1);
}

void	fsck_dir(t_fsck *k, uint32_t first)
{
	uint32_t	c;
	uint32_t	i;
	uint32_t	per;
	uint32_t	guard;

	if (first == 2)
		mark_chain(k, 2);
	per = k->g.bps * k->g.spc / 32;
	c = first;
	guard = 0;
	while (c >= 2 && c <= k->g.nclus + 1 && guard++ < 4096)
	{
		i = 0;
		while (i < per)
		{
			if (!dir_entry(k, fsck_clus(k, c) + i * 32))
				return ;
			i++;
		}
		c = fsck_fat(k, c);
	}
}

int	fake_dup_snames(const t_fakeblk *b)
{
	t_fsck		k;
	uint8_t		*e;
	uint32_t	i;
	uint32_t	j;
	int			dups;

	memset(&k, 0, sizeof(k));
	k.b = b;
	fakefat_geo(b, &k.g);
	e = fsck_clus(&k, 2);
	dups = 0;
	i = 0;
	while (i < k.g.bps * k.g.spc / 32 && e[i * 32])
	{
		j = i + 1;
		while (j < k.g.bps * k.g.spc / 32 && e[j * 32])
		{
			dups += (e[i * 32] != 0xE5 && (e[i * 32 + 11] & 0x3F) != 0x0F
					&& memcmp(e + i * 32, e + j * 32, 11) == 0);
			j++;
		}
		i++;
	}
	return (dups);
}
