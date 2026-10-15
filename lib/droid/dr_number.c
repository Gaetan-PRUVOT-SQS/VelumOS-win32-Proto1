#include "dr_int.h"

int	dr_int_tostring(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	uint16_t	buf[24];
	t_dstr16	s;

	s.p = buf;
	s.n = dr_dec16((int32_t)args[0], buf);
	return (dr_new16(vm, s, ret));
}

int	dr_str_concat(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	a;
	t_dstr16	b;
	uint16_t	*tmp;
	int			rc;

	rc = dr_str(vm, args[0], &a);
	if (rc == 0)
		rc = dr_str(vm, args[1], &b);
	if (rc != 0)
		return (rc);
	if (((uint64_t)a.n + b.n) * 2 > vm->lim.heap_bytes)
		return (dvm_throw(vm, DR_OOME, "chaîne trop longue"));
	tmp = malloc(((size_t)a.n + b.n + 1) * 2);
	if (!tmp)
		return (E_NOMEM);
	memcpy(tmp, a.p, (size_t)a.n * 2);
	memcpy(tmp + a.n, b.p, (size_t)b.n * 2);
	a.p = tmp;
	a.n += b.n;
	rc = dr_new16(vm, a, ret);
	free(tmp);
	return (rc);
}

static int	dr_digits(t_dstr16 s, uint32_t i, int64_t *v)
{
	if (i >= s.n)
		return (-1);
	while (i < s.n)
	{
		if (s.p[i] < '0' || s.p[i] > '9')
			return (-1);
		*v = *v * 10 + (s.p[i] - '0');
		if (*v > 2147483648LL)
			return (-1);
		i++;
	}
	return (0);
}

int	dr_int_parse(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;
	int64_t		v;
	uint32_t	neg;

	if (args[0] == DVM_NULL)
		return (dvm_throw(vm, DR_NFE, "null"));
	if (dr_str(vm, args[0], &s) != 0)
		return (DVM_THROWN);
	v = 0;
	neg = (s.n > 0 && s.p[0] == '-');
	if (dr_digits(s, neg || (s.n > 0 && s.p[0] == '+'), &v) != 0
		|| (!neg && v > 2147483647LL))
		return (dvm_throw(vm, DR_NFE, "entier invalide"));
	if (neg)
		v = -v;
	*ret = (uint32_t)(int32_t)v;
	return (0);
}
