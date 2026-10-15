#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

int	fdir_extend(t_fat *fs, t_fdirit *it)
{
	uint32_t	c;
	int			rc;

	rc = fat_alloc(fs, &c);
	if (rc < 0)
		return (rc);
	rc = fat_zero_clus(fs, c);
	if (rc < 0)
		return (rc);
	rc = fat_set(fs, c, FAT_EOC);
	if (rc < 0)
		return (rc);
	return (fat_set(fs, it->clus, c));
}

static int	run_step(t_fat *fs, t_fdirit *it)
{
	uint8_t		e[FAT_DENT];
	uint64_t	pos;
	int			rc;

	rc = fdir_pos(fs, it, it->idx, &pos);
	if (rc == E_RANGE)
		return (E_NOSPC);
	if (rc == E_NOENT)
		return (fdir_extend(fs, it));
	if (rc < 0)
		return (rc);
	rc = fdir_get(fs, pos, e);
	if (rc < 0)
		return (rc);
	it->run++;
	if (e[0] != 0 && e[0] != FAT_SLOT_FREE)
		it->run = 0;
	it->idx++;
	return (0);
}

static int	free_run(t_fat *fs, t_fdirit *it, uint32_t need)
{
	int	rc;

	while (it->run < need)
	{
		rc = run_step(fs, it);
		if (rc < 0)
			return (rc);
	}
	return (0);
}

static int	write_slots(t_fat *fs, uint32_t first, const t_fadd *a, t_fent *o)
{
	t_fdirit	it;
	uint8_t		e[FAT_DENT];
	uint32_t	k;
	int			rc;

	fdir_init(&it, first);
	k = 0;
	while (k <= a->nlfn)
	{
		rc = fdir_pos(fs, &it, a->start + k, &o->pos);
		if (rc < 0)
			return (rc);
		if (k < a->nlfn)
			fat_lfn_build(e, a, a->nlfn - k);
		else
			memcpy(e, a->raw, FAT_DENT);
		rc = fdir_put(fs, o->pos, e);
		if (rc < 0)
			return (rc);
		k++;
	}
	return (0);
}

int	fdir_add(t_fat *fs, uint32_t first, const char *name, t_fent *io)
{
	t_fadd		a;
	t_fdirit	it;
	int			rc;

	rc = fat_name_valid(name);
	if (rc == 0)
		rc = fat_pick_short(fs, first, name, &a);
	if (rc < 0)
		return (rc);
	memcpy(a.raw + 11, io->raw + 11, FAT_DENT - 11);
	a.raw[12] = 0;
	fdir_init(&it, first);
	rc = free_run(fs, &it, a.nlfn + 1);
	if (rc < 0)
		return (rc);
	a.start = it.idx - (a.nlfn + 1);
	rc = write_slots(fs, first, &a, io);
	if (rc < 0)
		return (rc);
	io->idx = a.start + a.nlfn;
	io->first_idx = a.start;
	memcpy(io->raw, a.raw, FAT_DENT);
	fat_sname_decode(a.raw, io->sname);
	strlcpy(io->name, name, sizeof(io->name));
	return (0);
}
