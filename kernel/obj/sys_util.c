#include "obj_int.h"

t_process	*sys_proc(void)
{
	t_process	*p;

	p = proc_current();
	if (!p || !p->handles)
		return (NULL);
	return (p);
}

int	sys_handle(uint64_t raw, uint32_t type, uint32_t need, t_hget *out)
{
	int	rc;

	out->obj = NULL;
	if (raw == HANDLE_INVALID || raw > UINT32_MAX)
		return (E_BADF);
	rc = handle_get(sys_proc(), (t_handle)raw, type, out);
	if (rc < 0)
		return (rc);
	return (handle_need(out, need));
}

int64_t	sys_install(t_object *o, uint32_t rights)
{
	t_handle	h;
	int			rc;

	rc = handle_alloc(sys_proc(), o, rights, &h);
	obj_unref(o);
	if (rc < 0)
		return (rc);
	return ((int64_t)h);
}
