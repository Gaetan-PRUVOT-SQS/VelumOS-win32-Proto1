#include "vfs_int.h"
#include "velum/err.h"
#include "velum/libk.h"

void	vfs_fill_stat(const t_vmount *m, const t_vnode *n, t_vstat *o)
{
	memset(o, 0, sizeof(*o));
	o->size = n->size;
	o->mtime_ns = n->mtime_ns;
	o->ctime_ns = n->ctime_ns;
	o->inode = n->ino;
	o->mode = n->mode;
	if ((m->flags & VFS_MOUNT_RO) || (n->attr & VFS_STAT_RO))
		o->flags = VFS_STAT_RO;
}

int	vfs_stat(const char *path, t_vstat *out)
{
	t_vres	r;
	t_vnode	n;
	int		rc;

	if (path == NULL || out == NULL)
		return (E_INVAL);
	rc = vfs_resolve(path, &r);
	if (rc < 0)
		return (rc);
	mutex_lock(&r.mnt->lock);
	rc = r.mnt->ops->lookup(r.mnt->fs, r.rel, &n);
	if (rc == 0)
		vfs_fill_stat(r.mnt, &n, out);
	mutex_unlock(&r.mnt->lock);
	vfs_mnt_put(r.mnt);
	return (rc);
}

int	vfs_fstat(struct s_vfile *f, t_vstat *out)
{
	if (f == NULL || out == NULL)
		return (E_BADF);
	mutex_lock(&f->mnt->lock);
	vfs_fill_stat(f->mnt, f->node, out);
	mutex_unlock(&f->mnt->lock);
	return (0);
}

int	vfs_fsync(struct s_vfile *f)
{
	int	rc;

	if (f == NULL)
		return (E_BADF);
	mutex_lock(&f->mnt->lock);
	rc = f->mnt->ops->sync(f->mnt->fs);
	mutex_unlock(&f->mnt->lock);
	return (rc);
}

int	vfs_truncate(struct s_vfile *f, uint64_t size)
{
	int	rc;

	if (f == NULL || (f->flags & VFS_ACC_MASK) == O_RDONLY)
		return (E_BADF);
	if (f->node->mode & S_TYPE_DIR)
		return (E_ISDIR);
	if (size > VFS_OFF_MAX)
		return (E_INVAL);
	if (f->mnt->flags & VFS_MOUNT_RO)
		return (E_PERM);
	mutex_lock(&f->mnt->lock);
	rc = f->mnt->ops->truncate(f->mnt->fs, f->node, size);
	mutex_unlock(&f->mnt->lock);
	return (rc);
}
