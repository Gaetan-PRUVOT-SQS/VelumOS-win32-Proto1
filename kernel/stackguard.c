#include "velum/panic.h"

_Noreturn void	__stack_chk_fail(void)
{
	panic("stack smashing detected (from %p)", __builtin_return_address(0));
}
