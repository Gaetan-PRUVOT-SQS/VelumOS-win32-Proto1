#include "blk_int.h"

#define ST_MAGIC "VELUMBLKTEST"
#define ST_MAGIC_LEN 12
#define ST_SPAN 192
#define ST_WRITE 144
#define ST_XOR 0x5a

static t_blkdev	*st_find(void)
{
	uint8_t		*sec;
	uint32_t	i;
	t_blkdev	*d;
	t_blkdev	*found;

	sec = kmalloc_tag(BLK_SS_MAX, HEAP_BLOCK);
	found = NULL;
	i = 0;
	while (sec && !found && i < blk_count())
	{
		d = blk_at(i++);
		if (d && d->parent && !blk_is_ro(d) && d->nsectors >= ST_SPAN
			&& blk_read(d, 0, 1, sec) == 0
			&& !memcmp(sec, ST_MAGIC, ST_MAGIC_LEN))
			found = d;
	}
	kfree(sec);
	if (!found)
		klog_err("block: autotest écriture : aucune partition de test");
	return (found);
}

static void	st_fill(uint8_t *b, uint32_t ss, uint32_t n, uint8_t x)
{
	uint64_t	k;
	uint64_t	s;

	k = 0;
	while (k < (uint64_t)n * ss)
	{
		s = 1 + k / ss;
		b[k] = (uint8_t)((s * 31 + k % ss) ^ x);
		k++;
	}
}

static int	st_cycle(t_blkdev *d, uint8_t *got, uint8_t *ref, uint8_t x)
{
	uint32_t	ss;

	ss = d->sector_size;
	st_fill(ref, ss, ST_WRITE, x);
	if (blk_write(d, 1, ST_WRITE, ref) < 0 || blk_flush(d) < 0)
		return (1);
	if (blk_read(d, 1, ST_WRITE, got) < 0)
		return (1);
	return (memcmp(got, ref, (size_t)ST_WRITE * ss) != 0);
}

static int	st_run(t_blkdev *d, uint8_t *got, uint8_t *ref)
{
	uint32_t	ss;

	ss = d->sector_size;
	if (blk_read(d, 1, ST_SPAN - 1, got) < 0)
		return (1);
	st_fill(ref, ss, ST_SPAN - 1, 0);
	if (memcmp(got, ref, (size_t)(ST_SPAN - 1) * ss))
	{
		klog_err("block: autotest %s : motif initial faux", d->name);
		return (1);
	}
	if (st_cycle(d, got, ref, ST_XOR) || st_cycle(d, got, ref, 0))
	{
		klog_err("block: autotest %s : relecture différente", d->name);
		return (1);
	}
	klog_info("block: autotest écriture %s OK", d->name);
	return (0);
}

int	blk_selftest_write(void)
{
	uint8_t		*got;
	uint8_t		*ref;
	t_blkdev	*d;
	int			f;

	d = st_find();
	if (!d)
		return (1);
	got = kmalloc_tag((size_t)ST_SPAN * d->sector_size, HEAP_BLOCK);
	ref = kmalloc_tag((size_t)ST_SPAN * d->sector_size, HEAP_BLOCK);
	f = 1;
	if (got && ref)
		f = st_run(d, got, ref);
	kfree(got);
	kfree(ref);
	return (f);
}
