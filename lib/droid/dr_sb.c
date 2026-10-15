#include "dr_int.h"

static t_drsb	*dr_sb(t_dvm *vm, t_dref ref)
{
	return (dvm_payload(vm, ref, dr_class(vm, DR_SB)));
}

int	dr_sb_put(t_dvm *vm, t_dref ref, t_dstr16 s)
{
	t_drsb	*sb;

	if (ref == DVM_NULL)
		return (dr_npe(vm));
	sb = dr_sb(vm, ref);
	if (!sb)
		return (dvm_throw(vm, DR_CCE, "StringBuilder attendu"));
	if (sb->len > DR_SB_CAP || s.n > DR_SB_CAP - sb->len)
		return (dvm_throw(vm, DR_OOME, "StringBuilder plein"));
	memmove(sb->buf + sb->len, s.p, (size_t)s.n * 2);
	sb->len += s.n;
	return (0);
}

int	dr_sb_ascii(t_dvm *vm, t_dref ref, const char *txt)
{
	uint16_t	buf[24];
	t_dstr16	s;

	s.p = buf;
	s.n = 0;
	while (txt[s.n] && s.n < 24)
	{
		buf[s.n] = (uint8_t)txt[s.n];
		s.n++;
	}
	return (dr_sb_put(vm, ref, s));
}

int	dr_sb_length(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_drsb	*sb;

	sb = dr_sb(vm, args[0]);
	if (!sb)
		return (dr_npe(vm));
	*ret = sb->len;
	return (0);
}

int	dr_sb_tostring(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_drsb		*sb;
	t_dstr16	s;

	sb = dr_sb(vm, args[0]);
	if (!sb)
		return (dr_npe(vm));
	s.p = sb->buf;
	s.n = sb->len;
	if (s.n > DR_SB_CAP)
		s.n = DR_SB_CAP;
	return (dr_new16(vm, s, ret));
}
