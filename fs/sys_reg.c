#include "fsys.h"
#include "vfs_weak.h"
#include "velum/klog.h"

static const t_fsysent	g_fsys[] = {
{SYS_OPEN, sys_open, "open"}, {SYS_READ, sys_read, "read"},
{SYS_WRITE, sys_write, "write"}, {SYS_PREAD, sys_pread, "pread"},
{SYS_PWRITE, sys_pwrite, "pwrite"}, {SYS_SEEK, sys_seek, "seek"},
{SYS_STAT, sys_stat, "stat"}, {SYS_FSTAT, sys_fstat, "fstat"},
{SYS_READDIR, sys_readdir, "readdir"}, {SYS_MKDIR, sys_mkdir, "mkdir"},
{SYS_UNLINK, sys_unlink, "unlink"}, {SYS_RENAME, sys_rename, "rename"},
{SYS_FSYNC, sys_fsync, "fsync"}, {SYS_TRUNCATE, sys_truncate, "truncate"},
{0, NULL, NULL}
};

static int	deps_ok(void)
{
	return (syscall_register && proc_current && obj_create && obj_unref
		&& handle_alloc && handle_get && copy_from_user && copy_to_user
		&& user_range_ok);
}

void	vfs_sys_register(void)
{
	uint32_t	i;
	int			rc;

	if (!deps_ok())
	{
		klog_warn("vfs: appels fichiers non branchés (lots absents)");
		return ;
	}
	fsys_quota_init();
	i = 0;
	while (g_fsys[i].fn)
	{
		rc = syscall_register(g_fsys[i].num, g_fsys[i].fn, g_fsys[i].name);
		if (rc < 0)
			klog_err("vfs: appel %s non enregistré (%d)", g_fsys[i].name,
				rc);
		i++;
	}
}
