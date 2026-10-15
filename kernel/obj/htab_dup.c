#include "obj_int.h"

static int	dup_into(t_process *d, t_hget *g, uint32_t rights, t_handle *out)
{
	int	rc;

	rc = E_PERM;
	if ((g->rights & HR_DUP) && (rights & ~g->rights) == 0)
		rc = handle_alloc(d, g->obj, rights, out);
	obj_unref(g->obj);
	return (rc);
}

int	handle_dup(t_process *s, t_handle h, t_process *d, t_handle *out)
{
	t_hget	g;
	int		rc;

	if (!out)
		return (E_INVAL);
	rc = handle_get(s, h, OBJ_NONE, &g);
	if (rc < 0)
		return (rc);
	return (dup_into(d, &g, g.rights, out));
}

int	handle_dup_as(t_process *p, t_handle h, uint32_t r, t_handle *o)
{
	t_hget	g;
	int		rc;

	if (!o || (r & ~HR_ALL))
		return (E_INVAL);
	rc = handle_get(p, h, OBJ_NONE, &g);
	if (rc < 0)
		return (rc);
	return (dup_into(p, &g, r, o));
}
