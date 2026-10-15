#include "blk_int.h"

static t_blkdev	*blk_root(t_blkdev *d, uint64_t *lba)
{
	if (!d->parent)
		return (d);
	*lba += d->first_lba;
	return (d->parent);
}

int	blk_read(t_blkdev *d, uint64_t lba, uint32_t n, void *buf)
{
	t_blkdev	*root;
	int			rc;

	if (!d || !buf)
		return (E_INVAL);
	rc = blk_range_ok(d, lba, n);
	if (rc < 0)
		return (rc);
	root = blk_root(d, &lba);
	return (root->ops->read(root, lba, n, buf));
}

int	blk_write(t_blkdev *d, uint64_t lba, uint32_t n, const void *buf)
{
	t_blkdev	*root;
	int			rc;

	if (!d || !buf)
		return (E_INVAL);
	if (blk_is_ro(d))
		return (E_PERM);
	rc = blk_range_ok(d, lba, n);
	if (rc < 0)
		return (rc);
	root = blk_root(d, &lba);
	return (root->ops->write(root, lba, n, buf));
}

int	blk_flush(t_blkdev *d)
{
	uint64_t	unused;
	t_blkdev	*root;

	if (!d)
		return (E_INVAL);
	unused = 0;
	root = blk_root(d, &unused);
	return (root->ops->flush(root));
}
