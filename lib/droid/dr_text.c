#include "dr_int.h"

int	dr_new16(t_dvm *vm, t_dstr16 s, uint64_t *ret)
{
	t_dref	r;
	int		rc;

	r = DVM_NULL;
	rc = dvm_string_utf16_new(vm, s, &r);
	if (rc == 0)
		*ret = r;
	return (rc);
}

int	dr_new8(t_dvm *vm, const char *s, uint64_t *ret)
{
	t_dref	r;
	int		rc;

	r = DVM_NULL;
	rc = dvm_string_utf8_new(vm, s, &r);
	if (rc == 0)
		*ret = r;
	return (rc);
}

int	dr_str(t_dvm *vm, t_dref ref, t_dstr16 *out)
{
	if (ref == DVM_NULL)
		return (dr_npe(vm));
	if (dvm_string_get(vm, ref, out) != 0)
		return (dvm_throw(vm, DR_CCE, "chaîne attendue"));
	return (0);
}

int	dr_cstr(t_dvm *vm, t_dref ref, char *out)
{
	t_dstr16	s;
	int			rc;

	rc = dr_str(vm, ref, &s);
	if (rc == 0)
		dr_utf8(s, out, DROID_TEXT_MAX);
	return (rc);
}

int	dr_text_of(t_dvm *vm, t_dref ref, t_dref *str)
{
	const t_dmethod	*m;
	t_dname			nm;
	t_dstr16		s;
	uint64_t		ret;
	int				rc;

	*str = ref;
	if (ref == DVM_NULL || dvm_string_get(vm, ref, &s) == 0)
		return (0);
	nm = (t_dname){"toString", "()Ljava/lang/String;"};
	*str = DVM_NULL;
	if (!dvm_class_of(vm, ref)
		|| dvm_method(vm, dvm_class_of(vm, ref), &nm, &m) != 0)
		return (dvm_throw(vm, DR_CCE, "toString absent"));
	ret = 0;
	rc = dvm_call(vm, m, &ref, &ret);
	if (rc != 0)
		return (rc);
	*str = (t_dref)ret;
	if (*str != DVM_NULL && dvm_string_get(vm, *str, &s) != 0)
		return (dvm_throw(vm, DR_CCE, "toString ne rend pas une chaîne"));
	return (0);
}
