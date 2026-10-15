#include "time_int.h"

int64_t	sys_time_mono(const t_sysargs *a)
{
	(void)a;
	return ((int64_t)time_now_ns());
}

int64_t	sys_time_wall(const t_sysargs *a)
{
	(void)a;
	return ((int64_t)time_wall_ns());
}

int64_t	sys_time_set_wall(const t_sysargs *a)
{
	int	rc;

	rc = a05_check_priv(a05_caller(), PF_ADMIN);
	if (rc < 0)
		return (rc);
	return (time_set_wall_ns(a->a[0]));
}
