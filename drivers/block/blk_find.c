#include "blk_int.h"

t_blkdev	*blk_find(const char *name)
{
	t_blkreg	*r;
	t_blkdev	*found;
	uint32_t	i;

	if (!name)
		return (NULL);
	r = blk_reg();
	found = NULL;
	mutex_lock(&r->lock);
	i = 0;
	while (i < r->n && !found)
	{
		if (!strncmp(r->devs[i]->name, name, BLK_NAME_MAX))
			found = r->devs[i];
		i++;
	}
	mutex_unlock(&r->lock);
	return (found);
}

bool	blk_registered(const t_blkdev *d)
{
	t_blkreg	*r;
	uint32_t	i;
	bool		found;

	r = blk_reg();
	found = false;
	mutex_lock(&r->lock);
	i = 0;
	while (i < r->n && !found)
	{
		found = (r->devs[i] == d);
		i++;
	}
	mutex_unlock(&r->lock);
	return (found);
}
