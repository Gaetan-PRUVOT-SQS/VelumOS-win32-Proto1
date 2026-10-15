#include "fat.h"
#include "vfs_weak.h"
#include "velum/err.h"
#include "velum/libk.h"

static t_fcslot	*slot_find(t_fat *fs, uint64_t sec)
{
	uint32_t	i;

	i = 0;
	while (i < FAT_CACHE_SLOTS)
	{
		if (fs->slot[i].valid && fs->slot[i].sec == sec)
			return (&fs->slot[i]);
		i++;
	}
	return (NULL);
}

static t_fcslot	*slot_victim(t_fat *fs)
{
	t_fcslot	*best;
	uint32_t	i;

	best = &fs->slot[0];
	i = 0;
	while (i < FAT_CACHE_SLOTS)
	{
		if (!fs->slot[i].valid)
			return (&fs->slot[i]);
		if (fs->slot[i].age < best->age)
			best = &fs->slot[i];
		i++;
	}
	return (best);
}

int	fat_bget(t_fat *fs, uint64_t sec, int mode, uint8_t **out)
{
	t_fcslot	*s;

	if (sec >= fs->tot_sec)
		return (E_IO);
	s = slot_find(fs, sec);
	if (s == NULL)
	{
		s = slot_victim(fs);
		s->valid = 0;
		if (mode != FC_ZERO
			&& blk_read(fs->dev, sec * fs->ratio, fs->ratio, s->buf) < 0)
			return (E_IO);
		s->sec = sec;
		s->valid = 1;
	}
	if (mode == FC_ZERO)
		memset(s->buf, 0, fs->bps);
	s->age = ++fs->tick;
	*out = s->buf;
	return (0);
}

int	fat_bput(t_fat *fs, uint64_t sec, const uint8_t *buf)
{
	if (fs->ro)
		return (E_PERM);
	if (sec >= fs->tot_sec)
		return (E_IO);
	if (blk_write(fs->dev, sec * fs->ratio, fs->ratio, buf) < 0)
	{
		fat_cache_drop(fs, sec, 1);
		return (E_IO);
	}
	return (0);
}

void	fat_cache_drop(t_fat *fs, uint64_t sec, uint64_t n)
{
	uint32_t	i;

	i = 0;
	while (i < FAT_CACHE_SLOTS)
	{
		if (fs->slot[i].valid && fs->slot[i].sec >= sec
			&& fs->slot[i].sec - sec < n)
			fs->slot[i].valid = 0;
		i++;
	}
}
