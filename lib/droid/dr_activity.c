#include "dr_int.h"

int	dr_act_content(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droidview	*v;
	int			rc;

	(void)ret;
	if (!dr_host(vm) || !dr_is(vm, args[0], DR_ACT))
		return (dr_npe(vm));
	rc = dr_view_get(vm, args[1], &v);
	if (rc != 0)
		return (rc);
	if (v->parent != 0)
		return (dvm_throw(vm, DR_ISE, "vue déjà placée"));
	dr_host(vm)->root = v->id;
	dr_host(vm)->dirty = 1;
	return (0);
}

int	dr_act_title(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droid		*d;
	t_dstr16	s;
	t_dref		str;
	int			rc;

	(void)ret;
	d = dr_host(vm);
	if (!d || !dr_is(vm, args[0], DR_ACT))
		return (dr_npe(vm));
	rc = dr_text_of(vm, args[1], &str);
	if (rc != 0)
		return (rc);
	d->title[0] = '\0';
	if (str != DVM_NULL && dvm_string_get(vm, str, &s) == 0)
		dr_utf8(s, d->title, DROID_TEXT_MAX);
	d->dirty = 1;
	return (0);
}

int	dr_act_finish(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)ret;
	if (!dr_host(vm) || !dr_is(vm, args[0], DR_ACT))
		return (dr_npe(vm));
	dr_host(vm)->finished = 1;
	return (0);
}
