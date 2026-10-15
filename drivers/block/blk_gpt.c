#include "blk_int.h"

static int	gpt_load_array(t_blkdev *d, const t_gpthdr *h, t_partlist *pl)
{
	uint8_t	*buf;
	int		rc;

	buf = kmalloc_tag((size_t)h->array_sectors * d->sector_size, HEAP_BLOCK);
	if (!buf)
		return (E_NOMEM);
	rc = blk_read(d, h->entry_lba, h->array_sectors, buf);
	if (rc == 0)
		rc = gpt_entries(buf, h, pl);
	kfree(buf);
	return (rc);
}

static int	gpt_load(t_blkdev *d, uint64_t lba, t_gpthdr *h, t_partlist *pl)
{
	uint8_t		*buf;
	t_gptgeo	g;
	int			rc;

	buf = kmalloc_tag(d->sector_size, HEAP_BLOCK);
	if (!buf)
		return (E_NOMEM);
	rc = blk_read(d, lba, 1, buf);
	g.ss = d->sector_size;
	g.nsectors = d->nsectors;
	g.lba = lba;
	if (rc == 0)
		rc = gpt_header(buf, &g, h);
	kfree(buf);
	if (rc < 0 || !pl)
		return (rc);
	return (gpt_load_array(d, h, pl));
}

int	gpt_scan(t_blkdev *d, t_partlist *pl)
{
	t_gpthdr	h;
	t_gpthdr	alt;
	int			rc;

	rc = gpt_load(d, 1, &h, pl);
	if (rc == 0)
	{
		if (gpt_load(d, h.alt_lba, &alt, NULL) < 0)
			klog_warn("block: %s: copie GPT de secours invalide", d->name);
		return (0);
	}
	if (rc == E_NOMEM)
		return (rc);
	klog_warn("block: %s: GPT principale invalide (%d), essai de la copie",
		d->name, rc);
	rc = gpt_load(d, d->nsectors - 1, &h, pl);
	if (rc < 0)
		return (rc);
	klog_warn("block: %s: copie GPT de secours utilisée", d->name);
	return (0);
}
