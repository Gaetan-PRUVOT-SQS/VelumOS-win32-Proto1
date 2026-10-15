#include "fsys.h"
#include "vfs_weak.h"
#include "velum/err.h"

int64_t	sys_seek(const t_sysargs *a)
{
	t_hget	hg;
	int64_t	rc;

	if (a->a[2] > SEEK_END_)
		return (E_INVAL);
	rc = fsys_get_any(a->a[0], &hg);
	if (rc < 0)
		return (rc);
	rc = vfs_seek(hg.obj->impl, (int64_t)a->a[1], (int)a->a[2]);
	obj_unref(hg.obj);
	return (rc);
}

int64_t	sys_stat(const t_sysargs *a)
{
	char	path[VFS_PATH_MAX];
	t_vstat	st;
	int		rc;

	if (proc_current == NULL || proc_current() == NULL)
		return (E_PERM);
	rc = fsys_path_in(a->a[0], a->a[1], path);
	if (rc == 0)
		rc = vfs_stat(path, &st);
	if (rc == 0 && copy_to_user(a->a[2], &st, sizeof(st)) < 0)
		rc = E_FAULT;
	return (rc);
}

int64_t	sys_fstat(const t_sysargs *a)
{
	t_hget	hg;
	t_vstat	st;
	int		rc;

	rc = fsys_get_any(a->a[0], &hg);
	if (rc < 0)
		return (rc);
	rc = vfs_fstat(hg.obj->impl, &st);
	obj_unref(hg.obj);
	if (rc == 0 && copy_to_user(a->a[1], &st, sizeof(st)) < 0)
		rc = E_FAULT;
	return (rc);
}

int64_t	sys_fsync(const t_sysargs *a)
{
	t_hget	hg;
	int		rc;

	rc = fsys_get_any(a->a[0], &hg);
	if (rc < 0)
		return (rc);
	rc = vfs_fsync(hg.obj->impl);
	obj_unref(hg.obj);
	return (rc);
}

int64_t	sys_truncate(const t_sysargs *a)
{
	t_hget	hg;
	int		rc;

	rc = fsys_get(a->a[0], OBJ_FILE, HR_WRITE, &hg);
	if (rc < 0)
		return (rc);
	rc = vfs_truncate(hg.obj->impl, a->a[1]);
	obj_unref(hg.obj);
	return (rc);
}
