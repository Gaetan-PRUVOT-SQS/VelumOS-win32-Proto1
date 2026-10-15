#include "fat.h"
#include "velum/libk.h"

static int	span_part(t_fat *fs, t_fspan *sp, uint32_t n)
{
	uint8_t		*b;
	uint64_t	sec;
	uint32_t	so;
	int			rc;

	sec = sp->at / fs->bps;
	so = (uint32_t)(sp->at % fs->bps);
	rc = fat_bget(fs, sec, FC_READ, &b);
	if (rc < 0)
		return (rc);
	if (!sp->write)
	{
		memcpy(sp->dst, b + so, n);
		sp->dst += n;
		return (0);
	}
	if (sp->src)
	{
		memcpy(b + so, sp->src, n);
		sp->src += n;
	}
	else
		memset(b + so, 0, n);
	return (fat_bput(fs, sec, b));
}

static int	span_full(t_fat *fs, t_fspan *sp, uint32_t nsec)
{
	uint64_t	sec;
	int			rc;

	sec = sp->at / fs->bps;
	if (!sp->write)
	{
		rc = fat_dread(fs, sec, nsec, sp->dst);
		sp->dst += (uint64_t)nsec * fs->bps;
		return (rc);
	}
	if (sp->src == NULL)
		return (fat_dwrite(fs, sec, nsec, fs->zero));
	rc = fat_dwrite(fs, sec, nsec, sp->src);
	sp->src += (uint64_t)nsec * fs->bps;
	return (rc);
}

static int	span_step(t_fat *fs, t_fspan *sp, uint64_t *n)
{
	uint32_t	so;
	int			rc;

	so = (uint32_t)(sp->at % fs->bps);
	if (so == 0 && sp->len >= fs->bps)
	{
		*n = sp->len / fs->bps;
		rc = span_full(fs, sp, (uint32_t)(*n));
		*n *= fs->bps;
		return (rc);
	}
	*n = fs->bps - so;
	if (*n > sp->len)
		*n = sp->len;
	return (span_part(fs, sp, (uint32_t)(*n)));
}

int	fat_span(t_fat *fs, t_fspan *sp)
{
	uint64_t	n;
	int			rc;

	while (sp->len)
	{
		rc = span_step(fs, sp, &n);
		if (rc < 0)
			return (rc);
		sp->at += n;
		sp->len -= n;
	}
	return (0);
}
