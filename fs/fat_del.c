#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

int	fdir_del(t_fat *fs, uint32_t first, const t_fent *ent)
{
	t_fdirit	it;
	uint8_t		e[FAT_DENT];
	uint64_t	pos;
	uint32_t	k;
	int			rc;

	fdir_init(&it, first);
	k = ent->first_idx;
	while (k <= ent->idx)
	{
		rc = fdir_pos(fs, &it, k, &pos);
		if (rc == 0)
			rc = fdir_get(fs, pos, e);
		if (rc < 0)
			return (rc);
		e[0] = FAT_SLOT_FREE;
		rc = fdir_put(fs, pos, e);
		if (rc < 0)
			return (rc);
		k++;
	}
	return (0);
}

int	fat_parent_of(t_fat *fs, uint32_t dir, uint32_t *parent)
{
	uint8_t	e[FAT_DENT];
	int		rc;

	if (!fat_ok(fs, dir))
		return (E_IO);
	rc = fdir_get(fs, fat_clus_byte(fs, dir) + FAT_DENT, e);
	if (rc < 0)
		return (rc);
	if (memcmp(e, "..         ", 11) != 0)
		return (E_IO);
	*parent = (le16(e + 20) << 16) | le16(e + 26);
	if (*parent == 0)
		*parent = fs->root;
	return (0);
}

int	fat_set_dotdot(t_fat *fs, uint32_t dir, uint32_t parent)
{
	uint8_t		e[FAT_DENT];
	uint64_t	pos;
	int			rc;

	if (!fat_ok(fs, dir))
		return (E_IO);
	pos = fat_clus_byte(fs, dir) + FAT_DENT;
	rc = fdir_get(fs, pos, e);
	if (rc < 0)
		return (rc);
	if (memcmp(e, "..         ", 11) != 0)
		return (E_IO);
	if (parent == fs->root)
		parent = 0;
	put16(e + 20, parent >> 16);
	put16(e + 26, parent & 0xFFFF);
	return (fdir_put(fs, pos, e));
}

static void	dot_fill(uint8_t *e, const char *name, uint32_t clus)
{
	memcpy(e, name, 11);
	e[11] = FAT_A_DIR;
	put16(e + 20, clus >> 16);
	put16(e + 26, clus & 0xFFFF);
}

int	fat_mkdir_clus(t_fat *fs, uint32_t parent, const uint8_t *tmpl,
		uint32_t *out)
{
	uint8_t		e[FAT_DENT];
	uint64_t	pos;
	int			rc;

	rc = fat_alloc(fs, out);
	if (rc == 0)
		rc = fat_zero_clus(fs, *out);
	if (rc < 0)
		return (rc);
	pos = fat_clus_byte(fs, *out);
	memcpy(e, tmpl, FAT_DENT);
	dot_fill(e, ".          ", *out);
	rc = fdir_put(fs, pos, e);
	if (parent == fs->root)
		parent = 0;
	dot_fill(e, "..         ", parent);
	if (rc == 0)
		rc = fdir_put(fs, pos + FAT_DENT, e);
	if (rc == 0)
		rc = fat_set(fs, *out, FAT_EOC);
	return (rc);
}
