#include "inp_sys.h"
#include "velum/kinput.h"

int	inp_sys_may_configure(void)
{
	t_process	*p;

	p = proc_current();
	return (p && (p->flags & PF_INPUT));
}
