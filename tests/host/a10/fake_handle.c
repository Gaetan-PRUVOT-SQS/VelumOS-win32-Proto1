#include "fake.h"
#include "velum/err.h"

void	fobj_reset(void)
{
	int	i;

	i = 0;
	while (i < FOBJ_HANDLES)
	{
		if (g_fobj.slot[i])
			obj_unref(g_fobj.slot[i]);
		g_fobj.slot[i] = NULL;
		i++;
	}
	g_fobj.fail_create = 0;
	g_fobj.fail_handle = 0;
}

int	handle_alloc(t_process *p, t_object *o, uint32_t rt, t_handle *out)
{
	int	i;

	(void)p;
	if (g_fobj.fail_handle)
		return (E_NOMEM);
	i = 0;
	while (i < FOBJ_HANDLES)
	{
		if (!g_fobj.slot[i])
		{
			obj_ref(o);
			g_fobj.slot[i] = o;
			g_fobj.rights[i] = rt;
			*out = (t_handle)(i + 1);
			return (0);
		}
		i++;
	}
	return (E_MFILE);
}

int	handle_get(t_process *p, t_handle h, uint32_t type, t_hget *out)
{
	(void)p;
	if (h == 0 || h > FOBJ_HANDLES || !g_fobj.slot[h - 1])
		return (E_BADF);
	if (g_fobj.slot[h - 1]->type != type)
		return (E_INVAL);
	obj_ref(g_fobj.slot[h - 1]);
	out->obj = g_fobj.slot[h - 1];
	out->rights = g_fobj.rights[h - 1];
	return (0);
}

int	handle_close(t_process *p, t_handle h)
{
	(void)p;
	if (h == 0 || h > FOBJ_HANDLES || !g_fobj.slot[h - 1])
		return (E_BADF);
	obj_unref(g_fobj.slot[h - 1]);
	g_fobj.slot[h - 1] = NULL;
	return (0);
}
