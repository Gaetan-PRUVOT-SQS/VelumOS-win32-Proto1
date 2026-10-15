#include "fat.h"
#include "vfs_weak.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/libk.h"

static int	read_bpb(t_fat *fs, t_blkdev *d)
{
	uint8_t		*buf;
	uint64_t	bytes;
	int			rc;

	if (__builtin_mul_overflow(d->nsectors, d->sector_size, &bytes))
		bytes = UINT64_MAX;
	buf = kmalloc_tag(d->sector_size, HEAP_FS);
	if (buf == NULL)
		return (E_NOMEM);
	rc = blk_read(d, 0, 1, buf);
	if (rc < 0)
		rc = E_IO;
	else
		rc = fat_bpb_parse(buf, bytes, d->sector_size, fs);
	kfree(buf);
	return (rc);
}

static int	cache_init(t_fat *fs)
{
	uint32_t	i;

	fs->slab = kmalloc_tag((size_t)FAT_CACHE_SLOTS * fs->bps + fs->clsz,
			HEAP_FS);
	if (fs->slab == NULL)
		return (E_NOMEM);
	i = 0;
	while (i < FAT_CACHE_SLOTS)
	{
		fs->slot[i].buf = fs->slab + (size_t)i * fs->bps;
		fs->slot[i].valid = 0;
		i++;
	}
	fs->zero = fs->slab + (size_t)FAT_CACHE_SLOTS * fs->bps;
	memset(fs->zero, 0, fs->clsz);
	return (0);
}

static int	read_info(t_fat *fs)
{
	uint8_t	*b;
	int		rc;

	fs->free_count = FAT_UNKNOWN;
	fs->next_free = 2;
	if (fs->fsinfo == 0)
		return (0);
	rc = fat_bget(fs, fs->fsinfo, FC_READ, &b);
	if (rc < 0)
		return (rc);
	if (le32(b) != FAT_FSI_LEAD || le32(b + 484) != FAT_FSI_STRUC
		|| le32(b + 508) != FAT_FSI_TRAIL)
	{
		fs->fsinfo = 0;
		return (0);
	}
	if (le32(b + 488) <= fs->nclus)
		fs->free_count = le32(b + 488);
	if (fat_ok(fs, le32(b + 492)))
		fs->next_free = le32(b + 492);
	return (0);
}

void	fat_op_release(void *fs)
{
	t_fat	*f;

	f = fs;
	if (f == NULL)
		return ;
	kfree(f->slab);
	kfree(f);
}

int	fat_mount(t_blkdev *d, uint32_t ro, t_fat **out)
{
	t_fat	*fs;
	int		rc;

	if (d == NULL || blk_read == NULL || blk_write == NULL
		|| d->sector_size < 512 || d->sector_size > 4096
		|| (d->sector_size & (d->sector_size - 1)))
		return (E_NOTSUP);
	fs = vfs_alloc(sizeof(*fs));
	if (fs == NULL)
		return (E_NOMEM);
	fs->dev = d;
	fs->ro = (ro || (d->flags & BLK_RO));
	rc = read_bpb(fs, d);
	if (rc == 0)
		rc = cache_init(fs);
	if (rc == 0)
		rc = read_info(fs);
	if (rc < 0)
	{
		fat_op_release(fs);
		return (rc);
	}
	*out = fs;
	return (0);
}
