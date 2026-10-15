#include "fat.h"
#include "velum/err.h"

int	fat_clus_at(t_fat *fs, t_vnode *n, uint32_t idx, uint32_t *out)
{
	uint32_t	v;
	int			rc;

	if (n->first == 0)
		return (E_NOENT);
	if (n->cur_clus == 0 || n->cur_idx > idx)
	{
		n->cur_idx = 0;
		n->cur_clus = n->first;
	}
	while (n->cur_idx < idx)
	{
		rc = fat_get(fs, n->cur_clus, &v);
		if (rc < 0)
			return (rc);
		if (v >= FAT_EOC_MIN)
			return (E_NOENT);
		if (!fat_ok(fs, v))
			return (E_IO);
		n->cur_clus = v;
		n->cur_idx++;
	}
	*out = n->cur_clus;
	return (0);
}

int	fat_clus_new(t_fat *fs, t_vnode *n, uint32_t idx, uint32_t *out)
{
	if (n->first == 0 && idx != 0)
		return (E_IO);
	if (n->first != 0 && n->cur_idx + 1 != idx)
		return (E_IO);
	return (fat_alloc(fs, out));
}

int	fat_link(t_fat *fs, t_vnode *n, uint32_t c)
{
	int	rc;

	rc = fat_set(fs, c, FAT_EOC);
	if (rc < 0)
		return (rc);
	if (n->first == 0)
	{
		n->first = c;
		n->cur_idx = 0;
		n->cur_clus = c;
		return (0);
	}
	rc = fat_set(fs, n->cur_clus, c);
	if (rc < 0)
		return (rc);
	n->cur_idx++;
	n->cur_clus = c;
	return (0);
}
