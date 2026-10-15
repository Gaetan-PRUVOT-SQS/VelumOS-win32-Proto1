#include "fat.h"
#include "velum/err.h"

static int	walk_step(t_fat *fs, t_fwalk *w)
{
	uint32_t	v;
	int			rc;

	rc = fat_get(fs, w->c, &v);
	if (rc < 0)
		return (rc);
	if (v >= FAT_EOC_MIN)
		return (0);
	if (!fat_ok(fs, v))
		return (E_IO);
	w->c = v;
	w->len++;
	w->lam++;
	if (w->c == w->saved || w->len > fs->nclus)
		return (E_IO);
	if (w->lam == w->power)
	{
		w->saved = w->c;
		w->power <<= 1;
		w->lam = 0;
	}
	return (1);
}

int	fat_chain_check(t_fat *fs, uint32_t first, uint64_t need)
{
	t_fwalk	w;
	int		rc;

	if (first == 0 && need == 0)
		return (0);
	if (!fat_ok(fs, first))
		return (E_IO);
	w.c = first;
	w.saved = first;
	w.power = 1;
	w.lam = 0;
	w.len = 1;
	rc = walk_step(fs, &w);
	while (rc > 0)
		rc = walk_step(fs, &w);
	if (rc < 0)
		return (rc);
	if (w.len < need)
		return (E_IO);
	return (0);
}

int	fat_free_chain(t_fat *fs, uint32_t c)
{
	uint32_t	v;
	uint32_t	n;
	int			rc;

	n = 0;
	while (n++ <= fs->nclus)
	{
		rc = fat_get(fs, c, &v);
		if (rc < 0)
			return (rc);
		rc = fat_set(fs, c, 0);
		if (rc < 0)
			return (rc);
		if (v >= FAT_EOC_MIN)
			return (0);
		if (!fat_ok(fs, v))
			return (E_IO);
		c = v;
	}
	return (E_IO);
}
