#include "dr_int.h"

int	dr_sb_append_obj(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;
	t_dref		str;
	int			rc;

	*ret = args[0];
	rc = dr_text_of(vm, args[1], &str);
	if (rc != 0)
		return (rc);
	if (str == DVM_NULL || dvm_string_get(vm, str, &s) != 0)
		return (dr_sb_ascii(vm, args[0], "null"));
	return (dr_sb_put(vm, args[0], s));
}

int	dr_sb_append_i(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	uint16_t	buf[24];
	t_dstr16	s;

	*ret = args[0];
	s.p = buf;
	s.n = dr_dec16((int32_t)args[1], buf);
	return (dr_sb_put(vm, args[0], s));
}

int	dr_sb_append_j(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	uint16_t	buf[24];
	t_dstr16	s;

	*ret = args[0];
	s.p = buf;
	s.n = dr_dec16((int64_t)(args[1] | ((uint64_t)args[2] << 32)), buf);
	return (dr_sb_put(vm, args[0], s));
}

int	dr_sb_append_c(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	uint16_t	c;
	t_dstr16	s;

	*ret = args[0];
	c = (uint16_t)args[1];
	s.p = &c;
	s.n = 1;
	return (dr_sb_put(vm, args[0], s));
}

int	dr_sb_append_z(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	*ret = args[0];
	if (args[1] != 0)
		return (dr_sb_ascii(vm, args[0], "true"));
	return (dr_sb_ascii(vm, args[0], "false"));
}
