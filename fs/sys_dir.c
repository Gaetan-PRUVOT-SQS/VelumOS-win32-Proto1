#include "fsys.h"
#include "vfs_weak.h"
#include "velum/err.h"

static int64_t	readdir_loop(t_vfile *f, t_uptr out, uint64_t max)
{
	t_dirent	d;
	uint64_t	i;
	int			rc;

	i = 0;
	rc = 0;
	while (i < max)
	{
		rc = vfs_readdir(f, &d);
		if (rc <= 0)
			break ;
		if (copy_to_user(out + i * sizeof(t_dirent), &d, sizeof(d)) < 0)
		{
			rc = E_FAULT;
			break ;
		}
		i++;
	}
	if (i)
		return ((int64_t)i);
	return (rc);
}

int64_t	sys_readdir(const t_sysargs *a)
{
	t_hget		hg;
	uint64_t	max;
	int64_t		rc;

	max = a->a[2];
	if (max > FSYS_DIRENTS_MAX)
		max = FSYS_DIRENTS_MAX;
	rc = fsys_get(a->a[0], OBJ_DIR, HR_READ, &hg);
	if (rc < 0)
		return (rc);
	if (max && !user_range_ok(a->a[1], max * sizeof(t_dirent)))
		rc = E_FAULT;
	else
		rc = readdir_loop(hg.obj->impl, a->a[1], max);
	obj_unref(hg.obj);
	return (rc);
}

int64_t	sys_mkdir(const t_sysargs *a)
{
	char		path[VFS_PATH_MAX];
	t_process	*p;
	int			rc;

	rc = fsys_writer(&p);
	if (rc == 0)
		rc = fsys_path_in(a->a[0], a->a[1], path);
	if (rc == 0)
		rc = vfs_mkdir(path, (uint32_t)a->a[2]);
	return (rc);
}

int64_t	sys_unlink(const t_sysargs *a)
{
	char		path[VFS_PATH_MAX];
	t_process	*p;
	int			rc;

	rc = fsys_writer(&p);
	if (rc == 0)
		rc = fsys_path_in(a->a[0], a->a[1], path);
	if (rc == 0)
		rc = vfs_unlink(path);
	return (rc);
}

int64_t	sys_rename(const t_sysargs *a)
{
	char		from[VFS_PATH_MAX];
	char		to[VFS_PATH_MAX];
	t_process	*p;
	int			rc;

	rc = fsys_writer(&p);
	if (rc == 0)
		rc = fsys_path_in(a->a[0], a->a[1], from);
	if (rc == 0)
		rc = fsys_path_in(a->a[2], a->a[3], to);
	if (rc == 0)
		rc = vfs_rename(from, to);
	return (rc);
}
