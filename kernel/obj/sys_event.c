#include "obj_int.h"

int64_t	sys_event_create(const t_sysargs *a)
{
	t_object	*o;
	int			rc;

	if (a->a[0] > 1 || a->a[1] > 1)
		return (E_INVAL);
	rc = evt_create((uint32_t)a->a[0], (uint32_t)a->a[1], &o);
	if (rc < 0)
		return (rc);
	return (sys_install(o, RIGHTS_SIGOBJ));
}

int64_t	sys_event_op(const t_sysargs *a)
{
	t_hget	g;
	int		rc;

	if (a->a[1] > EV_PULSE)
		return (E_INVAL);
	rc = sys_handle(a->a[0], OBJ_EVENT, HR_SIGNAL, &g);
	if (rc < 0)
		return (rc);
	rc = evt_op(g.obj, (uint32_t)a->a[1]);
	obj_unref(g.obj);
	return (rc);
}

int64_t	sys_timer_create(const t_sysargs *a)
{
	t_object	*o;
	int			rc;

	if (a->a[0] > 1)
		return (E_INVAL);
	rc = tmr_create((uint32_t)a->a[0], &o);
	if (rc < 0)
		return (rc);
	return (sys_install(o, RIGHTS_SIGOBJ));
}

int64_t	sys_timer_set(const t_sysargs *a)
{
	t_hget	g;
	int		rc;

	rc = sys_handle(a->a[0], OBJ_TIMER, HR_SIGNAL, &g);
	if (rc < 0)
		return (rc);
	rc = tmr_set(g.obj, a->a[1], a->a[2]);
	obj_unref(g.obj);
	return (rc);
}
