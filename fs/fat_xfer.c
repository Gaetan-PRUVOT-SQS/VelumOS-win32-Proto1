#include "fat.h"
#include "velum/err.h"

static void	xfer_span(t_fat *fs, t_fxfer *x, uint32_t c, t_fspan *sp)
{
	uint64_t	off;

	off = x->io->off + x->done;
	sp->at = fat_clus_byte(fs, c) + off % fs->clsz;
	sp->len = fs->clsz - off % fs->clsz;
	if (sp->len > x->io->len - x->done)
		sp->len = x->io->len - x->done;
	sp->dst = NULL;
	if (x->io->dst)
		sp->dst = x->io->dst + x->done;
	sp->src = NULL;
	if (x->io->src)
		sp->src = x->io->src + x->done;
	sp->write = x->write;
}

static int	xfer_get(t_fat *fs, t_vnode *n, t_fxfer *x, uint32_t *c)
{
	uint32_t	idx;
	int			rc;

	idx = (uint32_t)((x->io->off + x->done) / fs->clsz);
	rc = fat_clus_at(fs, n, idx, c);
	if (rc != E_NOENT)
		return (rc);
	if (!x->write)
		return (E_IO);
	rc = fat_clus_new(fs, n, idx, c);
	if (rc < 0)
		return (rc);
	return (1);
}

static int64_t	xfer_clus(t_fat *fs, t_vnode *n, t_fxfer *x)
{
	t_fspan		sp;
	uint64_t	len;
	uint32_t	c;
	int			fresh;
	int			rc;

	fresh = xfer_get(fs, n, x, &c);
	if (fresh < 0)
		return (fresh);
	xfer_span(fs, x, c, &sp);
	len = sp.len;
	rc = 0;
	if (fresh && len < fs->clsz)
		rc = fat_zero_clus(fs, c);
	if (rc == 0)
		rc = fat_span(fs, &sp);
	if (rc == 0 && fresh)
		rc = fat_link(fs, n, c);
	if (rc < 0)
		return (rc);
	return ((int64_t)len);
}

int64_t	fat_xfer(t_fat *fs, t_vnode *n, t_vio *io, int write)
{
	t_fxfer	x;
	int64_t	rc;

	x.io = io;
	x.done = 0;
	x.write = (uint32_t)write;
	fs->xerr = 0;
	while (x.done < io->len)
	{
		rc = xfer_clus(fs, n, &x);
		if (rc < 0)
		{
			fs->xerr = (int32_t)rc;
			if (x.done)
				return ((int64_t)x.done);
			return (rc);
		}
		x.done += (uint64_t)rc;
	}
	return ((int64_t)x.done);
}
