#include "dr_int.h"

int	dr_thr_init_msg(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dstr16	s;

	(void)ret;
	if (!dr_is(vm, args[0], DR_THR))
		return (dr_npe(vm));
	if (args[1] != DVM_NULL && dvm_string_get(vm, args[1], &s) != 0)
		return (dvm_throw(vm, DR_CCE, "message"));
	if (dvm_throwable_set_message(vm, args[0], args[1]) != 0)
		return (dr_npe(vm));
	return (0);
}

int	dr_thr_message(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_dref	msg;

	msg = DVM_NULL;
	if (dvm_throwable_message(vm, args[0], &msg) != 0)
		return (dr_npe(vm));
	*ret = msg;
	return (0);
}
