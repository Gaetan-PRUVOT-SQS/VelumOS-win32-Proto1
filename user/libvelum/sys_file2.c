#include "sys_int.h"
#include "velum/vfile.h"

int64_t	v_seek(t_handle file, int64_t offset, int whence)
{
	return (sys3(SYS_SEEK, file, (uint64_t)offset, (uint64_t)whence));
}

int	v_stat(const char *path, t_vstat *out)
{
	int64_t	path_len;

	path_len = sys_cstrlen(path, VFS_PATH_MAX);
	if (path_len < 0)
		return ((int)path_len);
	return ((int)sys3(SYS_STAT, sys_ptr(path), (uint64_t)path_len,
			sys_ptr(out)));
}

int	v_fstat(t_handle file, t_vstat *out)
{
	return ((int)sys2(SYS_FSTAT, file, sys_ptr(out)));
}

int	v_readdir(t_handle dir, t_dirent *out, uint32_t max)
{
	return ((int)sys3(SYS_READDIR, dir, sys_ptr(out), max));
}

int	v_mkdir(const char *path, uint32_t mode)
{
	int64_t	path_len;

	path_len = sys_cstrlen(path, VFS_PATH_MAX);
	if (path_len < 0)
		return ((int)path_len);
	return ((int)sys3(SYS_MKDIR, sys_ptr(path), (uint64_t)path_len, mode));
}
