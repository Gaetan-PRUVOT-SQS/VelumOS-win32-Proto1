#include "blk_int.h"

static int	name_valid(const char *name)
{
	size_t	len;
	size_t	i;
	char	c;

	len = strnlen(name, BLK_NAME_MAX);
	if (len == 0 || len >= BLK_NAME_MAX)
		return (0);
	i = 0;
	while (i < len)
	{
		c = name[i];
		if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')))
			return (0);
		i++;
	}
	return (1);
}

static int	geometry_valid(const t_blkdev *d)
{
	if (d->sector_size < BLK_SS_MIN || d->sector_size > BLK_SS_MAX)
		return (0);
	if (d->sector_size & (d->sector_size - 1))
		return (0);
	return (d->nsectors != 0);
}

int	blk_dev_valid(const t_blkdev *d)
{
	const t_blkdev	*p;

	if (!d || !name_valid(d->name) || !geometry_valid(d))
		return (E_INVAL);
	p = d->parent;
	if (!p)
	{
		if (!d->ops || !d->ops->read || !d->ops->write || !d->ops->flush)
			return (E_INVAL);
		if (d->first_lba != 0)
			return (E_INVAL);
		return (0);
	}
	if (p->parent || p->sector_size != d->sector_size)
		return (E_INVAL);
	if (d->first_lba >= p->nsectors
		|| d->nsectors > p->nsectors - d->first_lba)
		return (E_RANGE);
	return (0);
}

int	blk_range_ok(const t_blkdev *d, uint64_t lba, uint32_t n)
{
	if (n == 0)
		return (E_INVAL);
	if (lba >= d->nsectors || n > d->nsectors - lba)
		return (E_RANGE);
	return (0);
}

bool	blk_is_ro(const t_blkdev *d)
{
	if (d->flags & BLK_RO)
		return (true);
	return (d->parent && (d->parent->flags & BLK_RO));
}
