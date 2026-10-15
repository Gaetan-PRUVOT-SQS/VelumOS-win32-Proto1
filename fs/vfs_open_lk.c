#include "vfs_int.h"
#include "velum/err.h"

static int	open_check(const t_vnode *n, uint32_t fl)
{
	if (n->mode & S_TYPE_DIR)
	{
		if (fl & VFS_ACC_MASK)
			return (E_ISDIR);
		return (0);
	}
	if (fl & O_DIRECTORY)
		return (E_NOTDIR);
	if ((fl & VFS_ACC_MASK) && (n->attr & VFS_STAT_RO))
		return (E_ACCES);
	return (0);
}

static int	open_lookup(t_vres *r, uint32_t fl, t_vnode *tmp)
{
	const t_fsops	*ops;
	int				rc;

	ops = r->mnt->ops;
	rc = ops->lookup(r->mnt->fs, r->rel, tmp);
	if (rc == E_NOENT && (fl & O_CREAT))
		return (ops->create(r->mnt->fs, r->rel, S_TYPE_REG, tmp));
	if (rc == 0 && (fl & O_CREAT) && (fl & O_EXCL))
		return (E_EXIST);
	return (rc);
}

int	vfs_open_locked(t_vres *r, uint32_t fl, t_vnode **out)
{
	t_vnode	tmp;
	int		rc;

	if ((fl & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC))
		&& (r->mnt->flags & VFS_MOUNT_RO))
		return (E_PERM);
	rc = open_lookup(r, fl, &tmp);
	if (rc == 0)
		rc = open_check(&tmp, fl);
	if (rc == 0)
		rc = vnode_get(r->mnt, &tmp, out);
	if (rc == 0 && (fl & O_TRUNC))
	{
		rc = r->mnt->ops->truncate(r->mnt->fs, *out, 0);
		if (rc < 0)
			vnode_put(r->mnt, *out);
	}
	return (rc);
}
