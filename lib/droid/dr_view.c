#include "dr_int.h"

t_droidview	*dr_view_of(t_dvm *vm, t_dref ref)
{
	t_droid		*d;
	uint32_t	*id;

	d = dr_host(vm);
	id = dvm_payload(vm, ref, dr_class(vm, DR_VIEW));
	if (!d || !id || *id == 0 || *id > d->nviews
		|| d->views[*id - 1].self != ref)
		return (NULL);
	return (&d->views[*id - 1]);
}

int	dr_view_get(t_dvm *vm, t_dref ref, t_droidview **out)
{
	if (ref == DVM_NULL)
		return (dr_npe(vm));
	*out = dr_view_of(vm, ref);
	if (!*out)
		return (dvm_throw(vm, DR_CCE, "vue construite attendue"));
	return (0);
}

static uint32_t	dr_kind(t_dvm *vm, t_dref ref)
{
	if (dr_is(vm, ref, DR_BTN))
		return (DV_BUTTON);
	if (dr_is(vm, ref, DR_TV))
		return (DV_TEXT);
	if (dr_is(vm, ref, DR_LL))
		return (DV_LINEAR);
	return (DV_VIEW);
}

int	dr_view_init(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droid		*d;
	t_droidview	*v;
	uint32_t	*id;

	(void)ret;
	d = dr_host(vm);
	if (args[0] == DVM_NULL || !d)
		return (dr_npe(vm));
	id = dvm_payload(vm, args[0], dr_class(vm, DR_VIEW));
	if (!id)
		return (dvm_throw(vm, DR_CCE, "vue attendue"));
	if (dr_view_of(vm, args[0]))
		return (dvm_throw(vm, DR_ISE, "vue déjà construite"));
	if (d->nviews >= DROID_VIEWS_MAX)
		return (dvm_throw(vm, DR_OOME, "trop de vues"));
	if (dvm_pin(vm, args[0]) != 0)
		return (E_INVAL);
	v = &d->views[d->nviews++];
	memset(v, 0, sizeof(*v));
	v->kind = dr_kind(vm, args[0]);
	v->id = d->nviews;
	v->self = args[0];
	*id = v->id;
	return (0);
}

int	dr_view_listen(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droidview	*v;
	int			rc;

	(void)ret;
	rc = dr_view_get(vm, args[0], &v);
	if (rc != 0)
		return (rc);
	if (args[1] != DVM_NULL && !dr_is(vm, args[1], DR_OCL))
		return (dvm_throw(vm, DR_CCE, "écouteur attendu"));
	if (args[1] != DVM_NULL && dvm_pin(vm, args[1]) != 0)
		return (E_INVAL);
	dvm_unpin(vm, v->listener);
	v->listener = args[1];
	return (0);
}
