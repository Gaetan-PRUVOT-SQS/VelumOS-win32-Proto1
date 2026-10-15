#include "proc_int.h"
#include "velum/err.h"

int	proc_make_obj(t_process *p)
{
	if (!obj_process_new)
		return (0);
	p->obj = obj_process_new(p);
	if (!p->obj)
		return (E_NOMEM);
	return (0);
}

static void	*handle_target(t_object *o, uint32_t type)
{
	if (type == OBJ_PROCESS && obj_process_of)
		return (obj_process_of(o));
	if (type == OBJ_THREAD && obj_thread_of)
		return (obj_thread_of(o));
	return (NULL);
}

void	*proc_handle_obj(t_process *p, uint64_t h, uint32_t type, t_hget *out)
{
	void	*target;

	out->obj = NULL;
	out->rights = 0;
	if (!handle_get || !p || h == HANDLE_INVALID || h > UINT32_MAX)
		return (NULL);
	if (handle_get(p, (t_handle)h, type, out) < 0)
		return (NULL);
	target = handle_target(out->obj, type);
	if (target)
		return (target);
	obj_unref(out->obj);
	out->obj = NULL;
	return (NULL);
}

void	proc_obj_release(t_hget *g)
{
	if (g->obj)
		obj_unref(g->obj);
	g->obj = NULL;
}
