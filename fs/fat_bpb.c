#include "fat.h"
#include "velum/err.h"

static int	bpb_basic(const uint8_t *bs, uint32_t ss)
{
	uint32_t	bps;
	uint32_t	spc;

	bps = le16(bs + 11);
	spc = bs[13];
	if (!((bs[0] == 0xEB && bs[2] == 0x90) || bs[0] == 0xE9))
		return (E_INVAL);
	if (bs[510] != 0x55 || bs[511] != 0xAA)
		return (E_INVAL);
	if (bps != 512 && bps != 1024 && bps != 2048 && bps != 4096)
		return (E_INVAL);
	if (spc == 0 || (spc & (spc - 1)) || bps * spc > 65536)
		return (E_INVAL);
	if (le16(bs + 14) == 0 || bs[16] == 0)
		return (E_INVAL);
	if (bs[21] != 0xF0 && bs[21] < 0xF8)
		return (E_INVAL);
	if (bps < ss || bps % ss)
		return (E_NOTSUP);
	return (0);
}

static int	bpb_count(const uint8_t *bs, uint64_t bytes, t_fat *fs)
{
	uint64_t	fatsz;
	uint64_t	tot;
	uint64_t	used;
	uint64_t	nclus;

	fatsz = le16(bs + 22);
	if (fatsz == 0)
		fatsz = le32(bs + 36);
	tot = le16(bs + 19);
	if (tot == 0)
		tot = le32(bs + 32);
	used = le16(bs + 14) + (uint64_t)bs[16] * fatsz
		+ ((uint64_t)le16(bs + 17) * 32 + fs->bps - 1) / fs->bps;
	if (fatsz == 0 || tot <= used || tot > bytes / fs->bps)
		return (E_INVAL);
	nclus = (tot - used) / fs->spc;
	if (nclus < FAT_MIN_CLUS)
		return (E_NOTSUP);
	if (nclus > FAT_MAX_CLUS)
		return (E_INVAL);
	fs->tot_sec = tot;
	fs->fat_sz = (uint32_t)fatsz;
	fs->nclus = (uint32_t)nclus;
	fs->data_start = used;
	return (0);
}

static int	bpb_fat32(const uint8_t *bs, t_fat *fs)
{
	uint32_t	ext;

	if (le16(bs + 17) || le16(bs + 19) || le16(bs + 22))
		return (E_INVAL);
	if (le16(bs + 42))
		return (E_NOTSUP);
	if ((uint64_t)fs->fat_sz * fs->bps / 4 < (uint64_t)fs->nclus + 2)
		return (E_INVAL);
	ext = le16(bs + 40);
	fs->mirror = !(ext & 0x80);
	fs->active = 0;
	if (!fs->mirror)
		fs->active = ext & 0x0F;
	if (fs->active >= fs->nfats)
		return (E_INVAL);
	fs->root = le32(bs + 44);
	if (!fat_ok(fs, fs->root))
		return (E_INVAL);
	fs->fsinfo = le16(bs + 48);
	if (fs->fsinfo == 0xFFFF)
		fs->fsinfo = 0;
	if (fs->fsinfo >= fs->rsvd)
		return (E_INVAL);
	return (0);
}

int	fat_bpb_parse(const uint8_t *bs, uint64_t bytes, uint32_t ss, t_fat *fs)
{
	int	rc;

	rc = bpb_basic(bs, ss);
	if (rc < 0)
		return (rc);
	fs->bps = le16(bs + 11);
	fs->spc = bs[13];
	fs->clsz = fs->bps * fs->spc;
	fs->rsvd = le16(bs + 14);
	fs->nfats = bs[16];
	fs->fat_start = fs->rsvd;
	fs->ratio = fs->bps / ss;
	rc = bpb_count(bs, bytes, fs);
	if (rc < 0)
		return (rc);
	return (bpb_fat32(bs, fs));
}
