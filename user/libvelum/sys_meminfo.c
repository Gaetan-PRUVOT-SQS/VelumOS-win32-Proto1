#include "sys_int.h"
#include "velum/abi/abi_meminfo.h"
#include "velum/abi/abi_syscall.h"

int	v_meminfo(t_meminfo *out)
{
	return ((int)sys1(SYS_MEMINFO, sys_ptr(out)));
}
