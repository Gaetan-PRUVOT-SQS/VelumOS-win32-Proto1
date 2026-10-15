#include "blk_int.h"

static t_blkreg	g_reg;

t_blkreg	*blk_reg(void)
{
	if (!g_reg.ready)
	{
		mutex_init(&g_reg.lock, "blk.registry");
		g_reg.ready = true;
	}
	return (&g_reg);
}

static int	reg_insert(t_blkreg *r, t_blkdev *d)
{
	uint32_t	i;
	bool		parent_seen;

	i = 0;
	parent_seen = false;
	while (i < r->n)
	{
		if (r->devs[i] == d || !strcmp(r->devs[i]->name, d->name))
			return (E_EXIST);
		if (r->devs[i] == d->parent)
			parent_seen = true;
		i++;
	}
	if (d->parent && !parent_seen)
		return (E_INVAL);
	if (r->n >= BLK_MAX_DEVS)
		return (E_NOSPC);
	r->devs[r->n] = d;
	r->n++;
	return (0);
}

int	blk_register(t_blkdev *d)
{
	t_blkreg	*r;
	int			rc;

	rc = blk_dev_valid(d);
	if (rc < 0)
		return (rc);
	r = blk_reg();
	mutex_lock(&r->lock);
	rc = reg_insert(r, d);
	mutex_unlock(&r->lock);
	return (rc);
}

uint32_t	blk_count(void)
{
	t_blkreg	*r;
	uint32_t	n;

	r = blk_reg();
	mutex_lock(&r->lock);
	n = r->n;
	mutex_unlock(&r->lock);
	return (n);
}

t_blkdev	*blk_at(uint32_t i)
{
	t_blkreg	*r;
	t_blkdev	*d;

	r = blk_reg();
	d = NULL;
	mutex_lock(&r->lock);
	if (i < r->n)
		d = r->devs[i];
	mutex_unlock(&r->lock);
	return (d);
}
