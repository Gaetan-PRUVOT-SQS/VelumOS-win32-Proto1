#include "cpu_int.h"

extern void	proc_return_check(void) __attribute__((weak));

void	trap_leave_user(bool user)
{
	if (user && proc_return_check)
		proc_return_check();
	if (user)
		cpu_spec_user_return();
}
