#include "obj_int.h"

static void	wm_release(t_object **objs, uint32_t n)
{
	while (n > 0)
	{
		n--;
		obj_unref(objs[n]);
	}
}

static int	wm_collect(const t_sysargs *a, t_object **objs)
{
	t_handle	hs[WAIT_MAX];
	uint32_t	i;
	t_hget		g;
	int			rc;

	if (copy_from_user(hs, a->a[0], a->a[1] * sizeof(t_handle)) < 0)
		return (E_FAULT);
	i = 0;
	while (i < a->a[1])
	{
		rc = sys_handle(hs[i], OBJ_NONE, HR_WAIT, &g);
		if (rc < 0)
		{
			wm_release(objs, i);
			return (rc);
		}
		objs[i] = g.obj;
		i++;
	}
	return (0);
}

int64_t	sys_wait_many(const t_sysargs *a)
{
	t_object	*objs[WAIT_MAX];
	int			rc;

	if (a->a[1] == 0 || a->a[1] > WAIT_MAX || a->a[2] > WAIT_ALL)
		return (E_INVAL);
	rc = wm_collect(a, objs);
	if (rc < 0)
		return (rc);
	rc = wait_objects(objs, (uint32_t)a->a[1], (uint32_t)a->a[2], a->a[3]);
	wm_release(objs, (uint32_t)a->a[1]);
	return (rc);
}
