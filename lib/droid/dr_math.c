#include "dr_int.h"

int	dr_math_abs(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)vm;
	*ret = args[0];
	if ((int32_t)args[0] < 0)
		*ret = (uint32_t)(0u - args[0]);
	return (0);
}

int	dr_math_min(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)vm;
	*ret = args[0];
	if ((int32_t)args[1] < (int32_t)args[0])
		*ret = args[1];
	return (0);
}

int	dr_math_max(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)vm;
	*ret = args[0];
	if ((int32_t)args[1] > (int32_t)args[0])
		*ret = args[1];
	return (0);
}

int	dr_sys_millis(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droid	*d;

	(void)args;
	d = dr_host(vm);
	*ret = 0;
	if (d && d->clock)
		*ret = d->clock(d->user);
	return (0);
}
