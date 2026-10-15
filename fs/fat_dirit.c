#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

void	fdir_init(t_fdirit *it, uint32_t first)
{
	it->first = first;
	it->clus = first;
	it->cidx = 0;
	it->idx = 0;
	it->run = 0;
}

static int	dir_advance(t_fat *fs, t_fdirit *it, uint32_t want)
{
	uint32_t	next;
	int			rc;

	if (want < it->cidx)
	{
		it->clus = it->first;
		it->cidx = 0;
	}
	while (it->cidx < want)
	{
		rc = fat_get(fs, it->clus, &next);
		if (rc < 0)
			return (rc);
		if (next >= FAT_EOC_MIN)
			return (E_NOENT);
		if (!fat_ok(fs, next))
			return (E_IO);
		it->clus = next;
		it->cidx++;
	}
	return (0);
}

int	fdir_pos(t_fat *fs, t_fdirit *it, uint32_t idx, uint64_t *pos)
{
	int	rc;

	if (idx >= FAT_DIR_MAX)
		return (E_RANGE);
	if (!fat_ok(fs, it->first))
		return (E_IO);
	rc = dir_advance(fs, it, (uint32_t)((uint64_t)idx * FAT_DENT / fs->clsz));
	if (rc < 0)
		return (rc);
	*pos = fat_clus_byte(fs, it->clus) + (uint64_t)idx * FAT_DENT % fs->clsz;
	return (0);
}

int	fdir_get(t_fat *fs, uint64_t pos, uint8_t *e)
{
	uint8_t	*b;
	int		rc;

	rc = fat_bget(fs, pos / fs->bps, FC_READ, &b);
	if (rc < 0)
		return (rc);
	memcpy(e, b + pos % fs->bps, FAT_DENT);
	return (0);
}

int	fdir_put(t_fat *fs, uint64_t pos, const uint8_t *e)
{
	uint8_t	*b;
	int		rc;

	rc = fat_bget(fs, pos / fs->bps, FC_READ, &b);
	if (rc < 0)
		return (rc);
	memcpy(b + pos % fs->bps, e, FAT_DENT);
	return (fat_bput(fs, pos / fs->bps, b));
}
