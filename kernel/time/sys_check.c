#include "time_int.h"
#include "velum/err.h"

int	a05_check_priv(const t_process *p, uint32_t need)
{
	if (!p || (p->flags & need) != need)
		return (E_PERM);
	return (E_OK);
}

int	a05_check_power(uint64_t op)
{
	if (op != POWER_OFF && op != POWER_REBOOT)
		return (E_INVAL);
	return (E_OK);
}
