#include "sys_int.h"
#include "velum/vfile.h"

int64_t	v_open(const char *path, uint32_t flags, uint32_t mode)
{
	uint64_t	a[V_SYS_ARGS];
	int64_t		path_len;

	path_len = sys_cstrlen(path, VFS_PATH_MAX);
	if (path_len < 0)
		return (path_len);
	a[0] = sys_ptr(path);
	a[1] = (uint64_t)path_len;
	a[2] = flags;
	a[3] = mode;
	a[4] = 0;
	a[5] = 0;
	return (v_syscall6(SYS_OPEN, a));
}

int64_t	v_read(t_handle file, void *buf, size_t len)
{
	return (sys3(SYS_READ, file, sys_ptr(buf), len));
}

int64_t	v_write(t_handle file, const void *buf, size_t len)
{
	return (sys3(SYS_WRITE, file, sys_ptr(buf), len));
}

int64_t	v_pread(t_handle file, void *buf, size_t len, uint64_t offset)
{
	uint64_t	a[V_SYS_ARGS];

	a[0] = file;
	a[1] = sys_ptr(buf);
	a[2] = len;
	a[3] = offset;
	a[4] = 0;
	a[5] = 0;
	return (v_syscall6(SYS_PREAD, a));
}

int64_t	v_pwrite(t_handle file, const void *buf, size_t len, uint64_t offset)
{
	uint64_t	a[V_SYS_ARGS];

	a[0] = file;
	a[1] = sys_ptr(buf);
	a[2] = len;
	a[3] = offset;
	a[4] = 0;
	a[5] = 0;
	return (v_syscall6(SYS_PWRITE, a));
}
