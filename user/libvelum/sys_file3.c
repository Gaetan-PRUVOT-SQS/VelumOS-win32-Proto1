#include "sys_int.h"
#include "velum/vfile.h"

int	v_unlink(const char *path)
{
	int64_t	path_len;

	path_len = sys_cstrlen(path, VFS_PATH_MAX);
	if (path_len < 0)
		return ((int)path_len);
	return ((int)sys2(SYS_UNLINK, sys_ptr(path), (uint64_t)path_len));
}

int	v_rename(const char *old_path, const char *new_path)
{
	uint64_t	a[V_SYS_ARGS];
	int64_t		old_len;
	int64_t		new_len;

	old_len = sys_cstrlen(old_path, VFS_PATH_MAX);
	new_len = sys_cstrlen(new_path, VFS_PATH_MAX);
	if (old_len < 0 || new_len < 0)
		return (E_FAULT);
	a[0] = sys_ptr(old_path);
	a[1] = (uint64_t)old_len;
	a[2] = sys_ptr(new_path);
	a[3] = (uint64_t)new_len;
	a[4] = 0;
	a[5] = 0;
	return ((int)v_syscall6(SYS_RENAME, a));
}

int	v_fsync(t_handle file)
{
	return ((int)sys1(SYS_FSYNC, file));
}

int	v_truncate(t_handle file, uint64_t size)
{
	return ((int)sys2(SYS_TRUNCATE, file, size));
}
