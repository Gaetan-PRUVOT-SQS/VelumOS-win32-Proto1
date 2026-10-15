#include "obj_int.h"

static int	copy_name(const t_sysargs *a, char *buf)
{
	if (a->a[1] == 0 || a->a[1] > IPC_NAME_MAX || a->a[2] != 0)
		return (E_INVAL);
	if (copy_from_user(buf, a->a[0], a->a[1]) < 0)
		return (E_FAULT);
	if (!port_name_ok(buf, a->a[1]))
		return (E_INVAL);
	return (0);
}

int64_t	sys_port_listen(const t_sysargs *a)
{
	char		buf[IPC_NAME_MAX];
	t_process	*p;
	t_object	*o;
	int			rc;

	p = sys_proc();
	if (!p || !(p->flags & PF_LISTEN))
		return (E_PERM);
	rc = copy_name(a, buf);
	if (rc < 0)
		return (rc);
	rc = port_listen(buf, (uint32_t)a->a[1], &o);
	if (rc < 0)
		return (rc);
	return (sys_install(o, RIGHTS_LISTEN));
}

int64_t	sys_port_connect(const t_sysargs *a)
{
	char		buf[IPC_NAME_MAX];
	t_object	*o;
	int			rc;

	if (!sys_proc())
		return (E_PERM);
	rc = copy_name(a, buf);
	if (rc < 0)
		return (rc);
	rc = port_connect(buf, (uint32_t)a->a[1], &o);
	if (rc < 0)
		return (rc);
	return (sys_install(o, RIGHTS_CHAN));
}

static int	accept_reserved(t_htab *ht, t_object *lobj, uint64_t timeout,
		t_xfer *x)
{
	int	rc;

	x->n = 1;
	rc = htab_reserve(ht, x);
	if (rc < 0)
		return (rc);
	rc = port_accept(lobj, wait_deadline(timeout), &x->objs[0]);
	if (rc < 0)
		htab_unreserve(ht, x);
	x->rights[0] = RIGHTS_CHAN;
	return (rc);
}

int64_t	sys_port_accept(const t_sysargs *a)
{
	t_hget		g;
	t_xfer		x;
	t_htab		*ht;
	int			rc;

	rc = sys_handle(a->a[0], OBJ_LISTENER, HR_READ, &g);
	if (rc < 0)
		return (rc);
	ht = ht_of(sys_proc());
	rc = accept_reserved(ht, g.obj, a->a[1], &x);
	obj_unref(g.obj);
	if (rc < 0)
		return (rc);
	rc = htab_commit(ht, &x);
	if (rc < 0)
		return (rc);
	return ((int64_t)x.hs[0]);
}
