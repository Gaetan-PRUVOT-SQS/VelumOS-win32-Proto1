#include <string.h>
#include "fake.h"
#include "fat.h"

static void	boot_head(uint8_t *s, const t_fakegeo *g)
{
	s[0] = 0xEB;
	s[1] = 0x58;
	s[2] = 0x90;
	memcpy(s + 3, "VELUMTST", 8);
	put16(s + 11, g->bps);
	s[13] = (uint8_t)g->spc;
	put16(s + 14, g->rsvd);
	s[16] = (uint8_t)g->nfats;
	s[21] = 0xF8;
	put16(s + 24, 32);
	put16(s + 26, 64);
	put32(s + 32, (uint32_t)g->tot);
	put32(s + 36, g->fatsz);
	put32(s + 44, 2);
	put16(s + 48, 1);
	put16(s + 50, 6);
	s[64] = 0x80;
	s[66] = 0x29;
	put32(s + 67, 0x1234ABCD);
	memcpy(s + 71, "VELUMDATA  ", 11);
	memcpy(s + 82, "FAT32   ", 8);
	s[510] = 0x55;
	s[511] = 0xAA;
}

static void	fsinfo(uint8_t *f, const t_fakegeo *g)
{
	put32(f, FAT_FSI_LEAD);
	put32(f + 484, FAT_FSI_STRUC);
	put32(f + 488, g->nclus - 1);
	put32(f + 492, 3);
	put32(f + 508, FAT_FSI_TRAIL);
}

void	fakefat_geo(const t_fakeblk *b, t_fakegeo *g)
{
	const uint8_t	*s;

	s = b->img;
	g->bps = le16(s + 11);
	g->spc = s[13];
	g->rsvd = le16(s + 14);
	g->nfats = s[16];
	g->tot = le32(s + 32);
	g->fatsz = le32(s + 36);
	g->nclus = (uint32_t)((g->tot - g->rsvd - g->nfats * g->fatsz) / g->spc);
}

void	fakefat_set(t_fakeblk *b, uint32_t c, uint32_t v)
{
	t_fakegeo	g;
	uint32_t	i;

	fakefat_geo(b, &g);
	i = 0;
	while (i < g.nfats)
	{
		put32(b->img + ((uint64_t)g.rsvd + (uint64_t)i * g.fatsz) * g.bps
			+ (uint64_t)c * 4, v);
		i++;
	}
}

int	fakefat_format(t_fakeblk *b, uint32_t bps, uint32_t spc)
{
	t_fakegeo	g;

	g.bps = bps;
	g.spc = spc;
	g.rsvd = 32;
	g.nfats = 2;
	g.tot = b->dev.nsectors * b->dev.sector_size / bps;
	g.fatsz = (uint32_t)(((g.tot - g.rsvd) / spc + 2) * 4 / bps + 1);
	g.nclus = (uint32_t)((g.tot - g.rsvd - g.nfats * g.fatsz) / spc);
	memset(b->img, 0, b->dev.nsectors * b->dev.sector_size);
	boot_head(b->img, &g);
	memcpy(b->img + 6 * bps, b->img, 512);
	fsinfo(b->img + bps, &g);
	fsinfo(b->img + 7 * bps, &g);
	fakefat_set(b, 0, 0x0FFFFFF8);
	fakefat_set(b, 1, 0x0FFFFFFF);
	fakefat_set(b, 2, 0x0FFFFFFF);
	if (g.nclus < 65525)
		return (-1);
	return (0);
}
