#include "vfs_int.h"
#include "velum/err.h"

int	vfs_ns_begin(const char *path, t_vres *r)
{
	int	rc;

	if (path == NULL)
		return (E_INVAL);
	rc = vfs_resolve(path, r);
	if (rc < 0)
		return (rc);
	if (r->mnt->flags & VFS_MOUNT_RO)
		rc = E_PERM;
	else if (r->rel[0] == '\0')
		rc = E_BUSY;
	if (rc < 0)
		vfs_mnt_put(r->mnt);
	return (rc);
}

static int	ns_victim(t_vres *r, t_vnode *n)
{
	int	rc;

	rc = r->mnt->ops->lookup(r->mnt->fs, r->rel, n);
	if (rc == 0 && vnode_busy(r->mnt, n->ino))
		rc = E_BUSY;
	return (rc);
}

int	vfs_unlink(const char *path)
{
	t_vres	r;
	t_vnode	n;
	int		rc;

	rc = vfs_ns_begin(path, &r);
	if (rc < 0)
		return (rc);
	mutex_lock(&r.mnt->lock);
	rc = ns_victim(&r, &n);
	if (rc == 0)
		rc = r.mnt->ops->remove(r.mnt->fs, r.rel);
	mutex_unlock(&r.mnt->lock);
	vfs_mnt_put(r.mnt);
	return (rc);
}

static int	ns_move(t_vres *a, t_vres *b)
{
	t_vnode	n;
	int		rc;

	if (a->mnt != b->mnt)
		return (E_NOTSUP);
	mutex_lock(&a->mnt->lock);
	rc = ns_victim(a, &n);
	if (rc == 0)
		rc = a->mnt->ops->rename(a->mnt->fs, a->rel, b->rel);
	mutex_unlock(&a->mnt->lock);
	return (rc);
}

int	vfs_rename(const char *from, const char *to)
{
	t_vres	a;
	t_vres	b;
	int		rc;

	rc = vfs_ns_begin(from, &a);
	if (rc < 0)
		return (rc);
	rc = vfs_ns_begin(to, &b);
	if (rc < 0)
	{
		vfs_mnt_put(a.mnt);
		return (rc);
	}
	rc = ns_move(&a, &b);
	vfs_mnt_put(b.mnt);
	vfs_mnt_put(a.mnt);
	return (rc);
}
