#include "dr_int.h"

int	dr_str_length(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;
	int			rc;

	rc = dr_str(vm, args[0], &s);
	if (rc == 0)
		*ret = s.n;
	return (rc);
}

int	dr_str_charat(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;
	int			rc;

	rc = dr_str(vm, args[0], &s);
	if (rc != 0)
		return (rc);
	if (args[1] >= s.n)
		return (dvm_throw(vm, DR_SIOOBE, "indice hors de la chaîne"));
	*ret = s.p[args[1]];
	return (0);
}

int	dr_str_equals(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	a;
	t_dstr16	b;
	uint32_t	i;
	int			rc;

	rc = dr_str(vm, args[0], &a);
	if (rc != 0)
		return (rc);
	*ret = 0;
	if (dvm_string_get(vm, args[1], &b) != 0 || a.n != b.n)
		return (0);
	i = 0;
	while (i < a.n && a.p[i] == b.p[i])
		i++;
	*ret = (i == a.n);
	return (0);
}

int	dr_str_hash(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;
	uint32_t	h;
	uint32_t	i;
	int			rc;

	rc = dr_str(vm, args[0], &s);
	if (rc != 0)
		return (rc);
	h = 0;
	i = 0;
	while (i < s.n)
		h = h * 31u + s.p[i++];
	*ret = h;
	return (0);
}

int	dr_str_self(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;
	int			rc;

	rc = dr_str(vm, args[0], &s);
	if (rc == 0)
		*ret = args[0];
	return (rc);
}
