#include "vfs_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	mount_covers(const t_vmount *m, const char *norm)
{
	if (!m->used)
		return (0);
	if (m->len == 1)
		return (1);
	if (strncmp(norm, m->path, m->len) != 0)
		return (0);
	return (norm[m->len] == '\0' || norm[m->len] == '/');
}

static t_vmount	*mount_best(t_vfs *v, const char *norm)
{
	t_vmount	*best;
	uint32_t	i;

	best = NULL;
	i = 0;
	while (i < VFS_MOUNTS_MAX)
	{
		if (mount_covers(&v->mounts[i], norm)
			&& (best == NULL || v->mounts[i].len > best->len))
			best = &v->mounts[i];
		i++;
	}
	return (best);
}

int	vfs_resolve(const char *path, t_vres *res)
{
	t_vfs		*v;
	t_vmount	*m;
	int			rc;

	rc = vpath_norm(path, res->norm);
	if (rc < 0)
		return (rc);
	v = vfs_g();
	mutex_lock(&v->lock);
	m = mount_best(v, res->norm);
	if (m)
		m->users++;
	mutex_unlock(&v->lock);
	if (m == NULL)
		return (E_NOENT);
	res->mnt = m;
	res->rel = res->norm + m->len;
	while (*res->rel == '/')
		res->rel++;
	return (0);
}

void	vfs_mnt_put(t_vmount *m)
{
	t_vfs	*v;

	v = vfs_g();
	mutex_lock(&v->lock);
	if (m->users)
		m->users--;
	mutex_unlock(&v->lock);
}

int	vfs_mount(const char *target, const char *fs, t_blkdev *d, uint32_t fl)
{
	char	norm[VFS_PATH_MAX];
	int		rc;

	if (fs == NULL || (fl & ~(uint32_t)VFS_MOUNT_RO))
		return (E_INVAL);
	rc = vpath_norm(target, norm);
	if (rc < 0)
		return (rc);
	if (strcmp(fs, "fat32") == 0)
		return (vfs_fat_mount(norm, d, fl));
	if (strcmp(fs, "initrd") == 0)
		return (vfs_initrd_boot(norm));
	return (E_NOTSUP);
}
