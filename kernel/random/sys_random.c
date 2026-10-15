#include "rng_int.h"
#include "velum/err.h"
#include "velum/random.h"
#include "velum/vmm.h"

int64_t	sys_getrandom(const t_sysargs *a)
{
	uint8_t	buf[RANDOM_SYS_MAX];
	size_t	len;
	int		rc;

	if (a->a[2] != 0 || a->a[1] > RANDOM_SYS_MAX)
		return (E_INVAL);
	len = (size_t)a->a[1];
	if (!len)
		return (0);
	krandom(buf, len);
	rc = copy_to_user((t_uptr)a->a[0], buf, len);
	secure_zero(buf, sizeof(buf));
	if (rc < 0)
		return (rc);
	return ((int64_t)len);
}

int64_t	sys_sysinfo(const t_sysargs *a)
{
	t_sysinfo	si;
	int			rc;

	sysinfo_collect(&si);
	rc = copy_to_user((t_uptr)a->a[0], &si, sizeof(si));
	secure_zero(&si, sizeof(si));
	if (rc < 0)
		return (rc);
	return (0);
}
