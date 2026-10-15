#include "velum/ksyscall.h"
#include "velum/proc.h"

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	(void)num;
	(void)fn;
	(void)name;
	return (0);
}

int	proc_list(t_procinfo *out, uint32_t max)
{
	(void)out;
	(void)max;
	return (0);
}
