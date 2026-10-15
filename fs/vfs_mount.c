#include "vfs_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static t_vmount	*mount_exact(t_vfs *v, const char *norm)
{
	uint32_t	i;

	i = 0;
	while (i < VFS_MOUNTS_MAX)
	{
		if (v->mounts[i].used && strcmp(v->mounts[i].path, norm) == 0)
			return (&v->mounts[i]);
		i++;
	}
	return (NULL);
}

static int	mount_slot(t_vfs *v, const char *norm, t_vmount **out)
{
	uint32_t	i;

	if (mount_exact(v, norm))
		return (E_BUSY);
	i = 0;
	while (i < VFS_MOUNTS_MAX && v->mounts[i].used)
		i++;
	if (i == VFS_MOUNTS_MAX)
		return (E_NOSPC);
	*out = &v->mounts[i];
	return (0);
}

int	vfs_mount_add(const char *p, const t_fsops *o, void *s, uint32_t f)
{
	t_vfs		*v;
	t_vmount	*m;
	int			rc;

	v = vfs_g();
	mutex_lock(&v->lock);
	rc = mount_slot(v, p, &m);
	if (rc == 0)
	{
		strlcpy(m->path, p, VFS_PATH_MAX);
		m->len = (uint32_t)strlen(m->path);
		m->flags = f;
		m->ops = o;
		m->fs = s;
		m->users = 0;
		m->open = NULL;
		m->used = 1;
	}
	mutex_unlock(&v->lock);
	return (rc);
}

static int	umount_take(t_vfs *v, const char *norm, t_vmount *copy)
{
	t_vmount	*m;

	m = mount_exact(v, norm);
	if (m == NULL)
		return (E_INVAL);
	if (strcmp(norm, "/") == 0 || m->users || m->open)
		return (E_BUSY);
	copy->ops = m->ops;
	copy->fs = m->fs;
	m->used = 0;
	return (0);
}

int	vfs_umount(const char *target)
{
	char		norm[VFS_PATH_MAX];
	t_vmount	copy;
	t_vfs		*v;
	int			rc;

	rc = vpath_norm(target, norm);
	if (rc < 0)
		return (rc);
	v = vfs_g();
	mutex_lock(&v->lock);
	rc = umount_take(v, norm, &copy);
	mutex_unlock(&v->lock);
	if (rc < 0)
		return (rc);
	copy.ops->sync(copy.fs);
	copy.ops->release(copy.fs);
	return (0);
}
