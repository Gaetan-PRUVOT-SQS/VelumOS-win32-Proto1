#include "vfs_int.h"
#include "velum/err.h"
#include "velum/heap.h"

int	vfs_flags_ok(uint32_t fl)
{
	uint32_t	acc;

	acc = fl & VFS_ACC_MASK;
	if ((fl & ~(uint32_t)VFS_O_KNOWN) || acc == VFS_ACC_MASK)
		return (E_INVAL);
	if ((fl & O_EXCL) && !(fl & O_CREAT))
		return (E_INVAL);
	if ((fl & O_TRUNC) && acc == O_RDONLY)
		return (E_INVAL);
	if ((fl & O_DIRECTORY)
		&& (acc != O_RDONLY || (fl & (O_CREAT | O_TRUNC | O_APPEND))))
		return (E_INVAL);
	return (0);
}

static int	open_resolved(const char *path, uint32_t fl, t_vres *r,
		t_vnode **n)
{
	int	rc;

	rc = vfs_resolve(path, r);
	if (rc < 0)
		return (rc);
	mutex_lock(&r->mnt->lock);
	rc = vfs_open_locked(r, fl, n);
	mutex_unlock(&r->mnt->lock);
	if (rc < 0)
		vfs_mnt_put(r->mnt);
	return (rc);
}

int	vfs_open(const char *path, uint32_t flags, struct s_vfile **out)
{
	t_vres	r;
	t_vnode	*n;
	t_vfile	*f;
	int		rc;

	rc = vfs_flags_ok(flags);
	if (rc < 0 || path == NULL || out == NULL)
		return (E_INVAL);
	f = vfs_alloc(sizeof(*f));
	if (f == NULL)
		return (E_NOMEM);
	rc = open_resolved(path, flags, &r, &n);
	if (rc < 0)
	{
		kfree(f);
		return (rc);
	}
	f->mnt = r.mnt;
	f->node = n;
	f->flags = flags;
	mutex_init(&f->lock, "vfs.file");
	*out = f;
	return (0);
}

int	vfs_close(struct s_vfile *f)
{
	if (f == NULL)
		return (E_BADF);
	mutex_lock(&f->mnt->lock);
	vnode_put(f->mnt, f->node);
	mutex_unlock(&f->mnt->lock);
	vfs_mnt_put(f->mnt);
	kfree(f);
	return (0);
}
