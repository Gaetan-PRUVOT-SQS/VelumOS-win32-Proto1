#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	scan_entry(t_fat *fs, const uint8_t *e, t_fent *ent)
{
	if (e[0] == FAT_SLOT_FREE)
	{
		fs->lfn.ok = 0;
		return (0);
	}
	if ((e[11] & FAT_A_LFN_MASK) == FAT_A_LFN)
	{
		fat_lfn_feed(&fs->lfn, e, ent->idx);
		return (0);
	}
	if ((e[11] & FAT_A_VOL) || e[0] == '.')
	{
		fs->lfn.ok = 0;
		return (0);
	}
	memcpy(ent->raw, e, FAT_DENT);
	fat_sname_decode(e, ent->sname);
	ent->first_idx = ent->idx;
	if (fat_lfn_name(&fs->lfn, e, ent->name) == 0)
		ent->first_idx = fs->lfn.start;
	else
		strlcpy(ent->name, ent->sname, sizeof(ent->name));
	fs->lfn.ok = 0;
	return (1);
}

int	fdir_scan(t_fat *fs, t_fdirit *it, t_fent *ent)
{
	uint8_t	e[FAT_DENT];
	int		rc;

	fs->lfn.ok = 0;
	while (1)
	{
		rc = fdir_pos(fs, it, it->idx, &ent->pos);
		if (rc == E_NOENT || rc == E_RANGE)
			return (0);
		if (rc < 0)
			return (rc);
		rc = fdir_get(fs, ent->pos, e);
		if (rc < 0)
			return (rc);
		if (e[0] == 0)
			return (0);
		ent->idx = it->idx++;
		if (scan_entry(fs, e, ent))
			return (1);
	}
}

int	fdir_find(t_fat *fs, uint32_t first, const char *name, t_fent *o)
{
	t_fdirit	it;
	int			rc;

	fdir_init(&it, first);
	while (1)
	{
		rc = fdir_scan(fs, &it, o);
		if (rc == 0)
			return (E_NOENT);
		if (rc < 0)
			return (rc);
		if (vname_eq(o->name, name) || vname_eq(o->sname, name))
			return (0);
	}
}

int	fdir_empty(t_fat *fs, uint32_t first)
{
	t_fdirit	it;
	t_fent		e;
	int			rc;

	fdir_init(&it, first);
	rc = fdir_scan(fs, &it, &e);
	if (rc < 0)
		return (rc);
	if (rc)
		return (E_NOTEMPTY);
	return (0);
}
