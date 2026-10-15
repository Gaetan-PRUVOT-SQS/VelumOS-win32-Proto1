#include "blk_int.h"

static void	ebr_begin(t_ebr *c, const t_partlist *pl)
{
	memset(c, 0, sizeof(*c));
	c->ext_first = pl->ext_first;
	c->ext_count = pl->ext_count;
	c->cur = pl->ext_first;
	c->num = EBR_FIRST_NUM;
}

void	ebr_scan(t_blkdev *d, uint8_t *sec, t_partlist *pl)
{
	t_ebr	c;
	int		rc;

	if (pl->ext_overlap)
		klog_warn("block: %s: partition étendue sur une primaire, non suivie",
			d->name);
	else if (pl->extended > 1 || (pl->extended && !pl->ext_count))
		klog_warn("block: %s: partition étendue hors limites ou en trop, "
			"non suivie", d->name);
	if (!pl->ext_count)
		return ;
	ebr_begin(&c, pl);
	rc = 1;
	while (rc == 1 && c.links < EBR_LINKS_MAX)
	{
		rc = blk_read(d, c.cur, 1, sec);
		if (rc == 0)
			rc = ebr_step(sec, &c, pl);
		c.links++;
	}
	if (rc != 0)
		klog_warn("block: %s: chaîne étendue arrêtée au maillon %u (%d)",
			d->name, c.links, rc);
}
