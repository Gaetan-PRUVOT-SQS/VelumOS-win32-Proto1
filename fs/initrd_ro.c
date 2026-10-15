#include "cpio.h"
#include "velum/err.h"

int64_t	rd_write(void *fs, t_vnode *n, t_vio *io)
{
	(void)fs;
	(void)n;
	(void)io;
	return (E_PERM);
}

int	rd_create(void *fs, const char *rel, uint32_t t, t_vnode *o)
{
	(void)fs;
	(void)rel;
	(void)t;
	(void)o;
	return (E_PERM);
}

int	rd_remove(void *fs, const char *rel)
{
	(void)fs;
	(void)rel;
	return (E_PERM);
}

int	rd_rename(void *fs, const char *from, const char *to)
{
	(void)fs;
	(void)from;
	(void)to;
	return (E_PERM);
}

int	rd_truncate(void *fs, t_vnode *n, uint64_t size)
{
	(void)fs;
	(void)n;
	(void)size;
	return (E_PERM);
}
