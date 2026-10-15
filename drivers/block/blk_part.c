#include "blk_int.h"

static void	part_fill(t_blkdev *c, t_blkdev *d, const t_part *p)
{
	memset(c, 0, sizeof(*c));
	c->sector_size = d->sector_size;
	c->flags = d->flags;
	c->nsectors = p->count;
	c->first_lba = p->first;
	c->parent = d;
	c->ops = d->ops;
	c->priv = d->priv;
}

static int	part_register(t_blkdev *d, const t_part *p, const char *kind)
{
	t_blkdev	*c;
	int			rc;

	c = kmalloc_tag(sizeof(*c), HEAP_BLOCK);
	if (!c)
		return (E_NOMEM);
	part_fill(c, d, p);
	rc = blk_part_name(c->name, d->name, p->num);
	if (rc == 0)
		rc = blk_register(c);
	if (rc < 0)
	{
		kfree(c);
		return (rc);
	}
	klog_info("block: %s début %llu, %llu secteurs (%s)", c->name,
		(unsigned long long)p->first, (unsigned long long)p->count, kind);
	return (0);
}

static int	part_register_all(t_blkdev *d, const t_partlist *pl)
{
	uint32_t	i;
	int			rc;
	const char	*kind;

	kind = "mbr";
	if (pl->gpt)
		kind = "gpt";
	i = 0;
	while (i < pl->n)
	{
		rc = part_register(d, &pl->p[i], kind);
		if (rc < 0)
			return (rc);
		i++;
	}
	return ((int)pl->n);
}

static int	part_read_table(t_blkdev *d, t_partlist *pl)
{
	uint8_t	*sec;
	int		rc;

	sec = kmalloc_tag(d->sector_size, HEAP_BLOCK);
	if (!sec)
		return (E_NOMEM);
	rc = blk_read(d, 0, 1, sec);
	if (rc == 0)
		rc = mbr_parse(sec, d->nsectors, pl);
	if (rc == 0 && !pl->gpt)
		ebr_scan(d, sec, pl);
	kfree(sec);
	if (rc == 0 && pl->gpt)
		rc = gpt_scan(d, pl);
	return (rc);
}

int	blk_scan_partitions(t_blkdev *d)
{
	t_partlist	*pl;
	int			rc;

	if (!d || d->parent || !blk_registered(d))
		return (E_INVAL);
	pl = kmalloc_tag(sizeof(*pl), HEAP_BLOCK);
	if (!pl)
		return (E_NOMEM);
	rc = part_read_table(d, pl);
	if (rc == 0)
		rc = part_register_all(d, pl);
	kfree(pl);
	return (rc);
}
