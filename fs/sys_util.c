#include "fsys.h"
#include "vfs_weak.h"
#include "velum/err.h"
#include "velum/libk.h"

int	fsys_path_in(t_uptr p, uint64_t len, char *out)
{
	if (len == 0)
		return (E_INVAL);
	if (len >= VFS_PATH_MAX)
		return (E_RANGE);
	if (copy_from_user(out, p, (size_t)len) < 0)
		return (E_FAULT);
	out[len] = '\0';
	if (strnlen(out, (size_t)len) != len)
		return (E_INVAL);
	return (0);
}

int	fsys_writer(t_process **p)
{
	*p = proc_current();
	if (*p == NULL || !((*p)->flags & PF_FSWRITE))
		return (E_PERM);
	return (0);
}

int	fsys_get(uint64_t h, uint32_t type, uint32_t right, t_hget *hg)
{
	t_process	*p;
	int			rc;

	p = proc_current();
	if (p == NULL)
		return (E_PERM);
	if (h == 0 || h > UINT32_MAX)
		return (E_BADF);
	rc = handle_get(p, (t_handle)h, type, hg);
	if (rc < 0)
		return (rc);
	if ((hg->rights & right) != right)
	{
		obj_unref(hg->obj);
		return (E_PERM);
	}
	return (0);
}

int	fsys_get_any(uint64_t h, t_hget *hg)
{
	if (fsys_get(h, OBJ_FILE, 0, hg) == 0)
		return (0);
	return (fsys_get(h, OBJ_DIR, 0, hg));
}
