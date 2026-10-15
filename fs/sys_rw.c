#include "fsys.h"

int64_t	sys_read(const t_sysargs *a)
{
	return (fsys_rw(a, 0, 1));
}

int64_t	sys_write(const t_sysargs *a)
{
	return (fsys_rw(a, 1, 1));
}

int64_t	sys_pread(const t_sysargs *a)
{
	return (fsys_rw(a, 0, 0));
}

int64_t	sys_pwrite(const t_sysargs *a)
{
	return (fsys_rw(a, 1, 0));
}
