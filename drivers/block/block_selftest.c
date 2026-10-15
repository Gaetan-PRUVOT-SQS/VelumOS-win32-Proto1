#include "velum/boot.h"
#include "blk_int.h"

static int	st_device(t_blkdev *d)
{
	uint8_t	*buf;
	int		f;

	buf = kmalloc_tag(d->sector_size, HEAP_BLOCK);
	if (!buf)
		return (1);
	f = (blk_read(d, 0, 1, buf) != 0);
	f += (blk_read(d, d->nsectors - 1, 1, buf) != 0);
	f += (blk_read(d, d->nsectors, 1, buf) != E_RANGE);
	f += (blk_read(d, 0, 0, buf) != E_INVAL);
	f += (blk_read(d, 0, 1, NULL) != E_INVAL);
	if (blk_is_ro(d))
		f += (blk_write(d, 0, 1, buf) != E_PERM);
	kfree(buf);
	if (f)
		klog_err("block: autotest %s : %d échec(s)", d->name, f);
	return (f);
}

int	block_selftest(void)
{
	uint32_t	n;
	uint32_t	i;
	int			fails;

	n = blk_count();
	fails = 0;
	i = 0;
	while (i < n)
	{
		fails += st_device(blk_at(i));
		i++;
	}
	if (n == 0)
		klog_info("block: autotest sans disque");
	if (boot_cmdline_has("blktest"))
		fails += blk_selftest_write();
	return (fails);
}
