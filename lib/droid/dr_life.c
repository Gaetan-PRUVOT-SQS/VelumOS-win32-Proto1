#include "dr_int.h"

static int	dr_invoke(t_droid *d, t_dref self, const t_dname *nm, t_dref arg)
{
	const t_dmethod	*m;
	uint32_t		args[2];
	uint64_t		ret;
	t_dclass		*c;

	c = dvm_class_of(d->vm, self);
	if (!c)
		return (E_INVAL);
	if (dvm_method(d->vm, c, nm, &m) != 0)
		return (dr_missing(d, c, nm));
	args[0] = self;
	args[1] = arg;
	ret = 0;
	return (dr_fail(d, dvm_call(d->vm, m, args, &ret)));
}

static int	dr_cycle_up(t_droid *d)
{
	t_dname	nm;
	int		rc;

	nm = (t_dname){"<init>", "()V"};
	rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	nm = (t_dname){"onCreate", "(Landroid/os/Bundle;)V"};
	if (rc == 0)
		rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	nm = (t_dname){"onStart", "()V"};
	if (rc == 0)
		rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	nm = (t_dname){"onResume", "()V"};
	if (rc == 0)
		rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	return (rc);
}

int	droid_start(t_droid *d, const char *activity_desc)
{
	t_dclass	*c;
	t_dref		act;
	int			rc;

	if (!d || !d->vm || !activity_desc || d->activity != DVM_NULL)
		return (E_INVAL);
	rc = dvm_class(d->vm, activity_desc, &c);
	if (rc == 0)
		rc = dvm_new(d->vm, c, &act);
	if (rc != 0)
		return (dr_fail(d, rc));
	if (!dr_is(d->vm, act, DR_ACT) || dvm_pin(d->vm, act) != 0)
		return (dr_fail(d, dvm_throw(d->vm, DR_CCE, activity_desc)));
	d->activity = act;
	return (dr_cycle_up(d));
}

int	droid_click(t_droid *d, uint32_t view_id)
{
	const t_droidview	*v;
	t_dname				nm;

	v = droid_view(d, view_id);
	if (!v || !d->vm || d->activity == DVM_NULL)
		return (E_INVAL);
	if (v->listener == DVM_NULL)
		return (0);
	nm = (t_dname){"onClick", "(Landroid/view/View;)V"};
	return (dr_invoke(d, v->listener, &nm, v->self));
}

int	droid_stop(t_droid *d)
{
	t_dname	nm;
	int		rc;

	if (!d || !d->vm || d->activity == DVM_NULL)
		return (E_INVAL);
	nm = (t_dname){"onPause", "()V"};
	rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	nm = (t_dname){"onStop", "()V"};
	if (rc == 0)
		rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	nm = (t_dname){"onDestroy", "()V"};
	if (rc == 0)
		rc = dr_invoke(d, d->activity, &nm, DVM_NULL);
	dr_release(d);
	return (rc);
}
