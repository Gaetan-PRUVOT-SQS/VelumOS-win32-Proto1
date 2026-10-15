#include "fat.h"
#include "velum/err.h"

static int	shrink_all(t_fat *fs, t_vnode *n)
{
	uint32_t	old;
	int			rc;

	old = n->first;
	n->first = 0;
	n->size = 0;
	n->cur_idx = 0;
	n->cur_clus = 0;
	rc = fat_node_sync(fs, n, 1);
	if (rc < 0 || old == 0)
		return (rc);
	return (fat_free_chain(fs, old));
}

static int	shrink_part(t_fat *fs, t_vnode *n, uint64_t size, uint32_t keep)
{
	uint32_t	last;
	uint32_t	next;
	int			rc;

	rc = fat_clus_at(fs, n, keep - 1, &last);
	if (rc == E_NOENT)
		return (E_IO);
	if (rc == 0)
		rc = fat_get(fs, last, &next);
	if (rc < 0)
		return (rc);
	n->size = size;
	rc = fat_node_sync(fs, n, 1);
	if (rc == 0 && next < FAT_EOC_MIN)
		rc = fat_set(fs, last, FAT_EOC);
	if (rc == 0 && next < FAT_EOC_MIN)
		rc = fat_free_chain(fs, next);
	n->cur_idx = 0;
	n->cur_clus = 0;
	return (rc);
}

int	fat_op_truncate(void *fs, t_vnode *n, uint64_t size)
{
	t_fat		*f;
	uint64_t	keep;
	int			rc;

	f = fs;
	if (f->ro)
		return (E_PERM);
	if (n->mode & S_TYPE_DIR)
		return (E_ISDIR);
	if (size > FAT_FILE_MAX)
		return (E_RANGE);
	keep = (size + f->clsz - 1) / f->clsz;
	rc = 0;
	if (size > n->size)
		rc = fat_fill_zero(f, n, size);
	else if (size < n->size && keep == 0)
		return (fat_commit(f, shrink_all(f, n)));
	else if (size < n->size)
		return (fat_commit(f, shrink_part(f, n, size, (uint32_t)keep)));
	if (fat_node_sync(f, n, 1) < 0 && rc == 0)
		rc = E_IO;
	return (fat_commit(f, rc));
}
