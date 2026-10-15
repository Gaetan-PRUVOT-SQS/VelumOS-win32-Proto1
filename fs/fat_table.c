#include "fat.h"
#include "velum/err.h"

static uint64_t	entry_sec(const t_fat *fs, uint32_t copy, uint32_t c,
		uint32_t *off)
{
	uint64_t	byte;

	byte = (uint64_t)c * 4;
	*off = (uint32_t)(byte % fs->bps);
	return (fs->fat_start + (uint64_t)copy * fs->fat_sz + byte / fs->bps);
}

int	fat_get(t_fat *fs, uint32_t c, uint32_t *v)
{
	uint8_t		*b;
	uint64_t	sec;
	uint32_t	off;
	int			rc;

	if (!fat_ok(fs, c))
		return (E_IO);
	sec = entry_sec(fs, fs->active, c, &off);
	rc = fat_bget(fs, sec, FC_READ, &b);
	if (rc < 0)
		return (rc);
	*v = le32(b + off) & FAT_MASK;
	return (0);
}

static int	set_copy(t_fat *fs, uint32_t copy, uint32_t c, uint32_t v)
{
	uint8_t		*b;
	uint64_t	sec;
	uint32_t	off;
	int			rc;

	sec = entry_sec(fs, copy, c, &off);
	rc = fat_bget(fs, sec, FC_READ, &b);
	if (rc < 0)
		return (rc);
	put32(b + off, (le32(b + off) & FAT_HIGH) | (v & FAT_MASK));
	return (fat_bput(fs, sec, b));
}

int	fat_set(t_fat *fs, uint32_t c, uint32_t v)
{
	uint32_t	old;
	uint32_t	i;
	int			rc;

	rc = fat_get(fs, c, &old);
	i = 0;
	while (rc == 0 && i < fs->nfats)
	{
		if (fs->mirror || i == fs->active)
			rc = set_copy(fs, i, c, v);
		i++;
	}
	if (rc < 0 || fs->free_count == FAT_UNKNOWN || (old == 0) == (v == 0))
		return (rc);
	if (v != 0 && fs->free_count > 0)
		fs->free_count--;
	else if (v == 0 && fs->free_count < fs->nclus)
		fs->free_count++;
	else
		return (0);
	fs->info_dirty = 1;
	return (0);
}

int	fat_alloc(t_fat *fs, uint32_t *out)
{
	uint32_t	c;
	uint32_t	v;
	uint32_t	n;
	int			rc;

	c = fs->next_free;
	n = 0;
	while (n < fs->nclus)
	{
		if (!fat_ok(fs, c))
			c = 2;
		rc = fat_get(fs, c, &v);
		if (rc < 0)
			return (rc);
		if (v == 0)
		{
			*out = c;
			fs->info_dirty |= (fs->next_free != c + 1);
			fs->next_free = c + 1;
			return (0);
		}
		c++;
		n++;
	}
	return (E_NOSPC);
}
