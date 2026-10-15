#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"

static int	ram_read(t_blkdev *d, uint64_t lba, uint32_t n, void *buf)
{
	t_fkram	*r;

	r = d->priv;
	if (d->parent || lba > d->nsectors || n > d->nsectors - lba)
		abort();
	r->reads++;
	r->last_lba = lba;
	r->last_n = n;
	if (r->fail_rc)
		return (r->fail_rc);
	if (r->fail_at && lba == r->fail_at)
		return (E_IO);
	memcpy(buf, r->data + lba * d->sector_size, (size_t)n * d->sector_size);
	return (0);
}

static int	ram_write(t_blkdev *d, uint64_t lba, uint32_t n, const void *buf)
{
	t_fkram	*r;

	r = d->priv;
	if (d->parent || lba > d->nsectors || n > d->nsectors - lba)
		abort();
	r->writes++;
	r->last_lba = lba;
	r->last_n = n;
	if (r->fail_rc)
		return (r->fail_rc);
	memcpy(r->data + lba * d->sector_size, buf, (size_t)n * d->sector_size);
	return (0);
}

static int	ram_flush(t_blkdev *d)
{
	t_fkram	*r;

	r = d->priv;
	if (d->parent)
		abort();
	r->flushes++;
	return (r->fail_rc);
}

void	fk_ram_init(t_fkram *r, const char *nm, uint32_t ss, uint64_t ns)
{
	static const t_blkops	ops = {ram_read, ram_write, ram_flush};

	memset(r, 0, sizeof(*r));
	snprintf(r->dev.name, sizeof(r->dev.name), "%s", nm);
	r->dev.sector_size = ss;
	r->dev.nsectors = ns;
	r->dev.ops = &ops;
	r->dev.priv = r;
	r->data = calloc(ns, ss);
	if (!r->data)
		abort();
}

void	fk_ram_free(t_fkram *r)
{
	free(r->data);
	r->data = NULL;
}
