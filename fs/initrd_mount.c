#include "cpio.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"

static const t_fsops	g_initrd_ops = {
	"initrd", rd_lookup, rd_open, rd_read, rd_write, rd_readdir, rd_create,
	rd_remove, rd_rename, rd_truncate, rd_sync, rd_release
};

int	rd_open(void *fs, t_vnode *n)
{
	(void)fs;
	(void)n;
	return (0);
}

int	rd_sync(void *fs)
{
	(void)fs;
	return (0);
}

void	rd_release(void *fs)
{
	t_initrd	*rd;

	rd = fs;
	if (rd == NULL)
		return ;
	kfree(rd->ents);
	kfree(rd);
}

int	vfs_initrd_mount(const char *t, const void *b, uint64_t sz)
{
	t_initrd	*rd;
	int			rc;

	rd = vfs_alloc(sizeof(*rd));
	if (rd == NULL)
		return (E_NOMEM);
	rc = cpio_index(b, sz, rd);
	if (rc == 0)
		rc = vfs_mount_add(t, &g_initrd_ops, rd, VFS_MOUNT_RO);
	if (rc < 0)
	{
		rd_release(rd);
		return (rc);
	}
	klog_info("vfs: initrd sur %s, %u entrées, %u ignorées", t, rd->n,
		rd->skipped);
	return (0);
}

int	vfs_mount_initrd(const char *target, const void *base, uint64_t sz)
{
	char	norm[VFS_PATH_MAX];
	int		rc;

	if (target == NULL)
		return (E_INVAL);
	rc = vpath_norm(target, norm);
	if (rc < 0)
		return (rc);
	return (vfs_initrd_mount(norm, base, sz));
}
