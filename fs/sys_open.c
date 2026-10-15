#include "fsys.h"
#include "velum/err.h"

static int	open_needs_write(uint32_t fl)
{
	return ((fl & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC)) != 0);
}

static int	open_args(const t_sysargs *a, t_process **p, char *path)
{
	int	rc;

	*p = proc_current();
	if (*p == NULL)
		return (E_PERM);
	if (a->a[2] > UINT32_MAX)
		return (E_INVAL);
	rc = fsys_path_in(a->a[0], a->a[1], path);
	if (rc < 0)
		return (rc);
	if (open_needs_write((uint32_t)a->a[2]) && !((*p)->flags & PF_FSWRITE))
		return (E_PERM);
	return (0);
}

int64_t	sys_open(const t_sysargs *a)
{
	char		path[VFS_PATH_MAX];
	t_process	*p;
	t_vfile		*f;
	t_handle	h;
	int			rc;

	rc = open_args(a, &p, path);
	if (rc == 0)
		rc = fsys_quota_take(p->pid);
	if (rc < 0)
		return (rc);
	rc = vfs_open(path, (uint32_t)a->a[2], &f);
	if (rc < 0)
	{
		fsys_quota_drop(p->pid);
		return (rc);
	}
	f->owner = p->pid;
	rc = fsys_handle_new(p, f, &h);
	if (rc < 0)
		return (rc);
	return (h);
}
