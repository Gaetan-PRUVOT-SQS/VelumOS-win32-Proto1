#include "obj_int.h"

int64_t	sys_close(const t_sysargs *a)
{
	if (a->a[0] == HANDLE_INVALID || a->a[0] > UINT32_MAX)
		return (E_BADF);
	return (handle_close(sys_proc(), (t_handle)a->a[0]));
}

int64_t	sys_dup(const t_sysargs *a)
{
	t_handle	h;
	int			rc;

	if (a->a[0] == HANDLE_INVALID || a->a[0] > UINT32_MAX)
		return (E_BADF);
	if (a->a[1] & ~(uint64_t)HR_ALL)
		return (E_INVAL);
	rc = handle_dup_as(sys_proc(), (t_handle)a->a[0], (uint32_t)a->a[1], &h);
	if (rc < 0)
		return (rc);
	return ((int64_t)h);
}

int64_t	sys_wait(const t_sysargs *a)
{
	t_hget	g;
	int		rc;

	rc = sys_handle(a->a[0], OBJ_NONE, HR_WAIT, &g);
	if (rc < 0)
		return (rc);
	rc = obj_wait(g.obj, a->a[1]);
	obj_unref(g.obj);
	return (rc);
}
