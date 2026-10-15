#ifndef FSYS_H
# define FSYS_H

# include <stdint.h>
# include "vfs_int.h"
# include "velum/ksyscall.h"
# include "velum/object.h"
# include "velum/proc.h"

# define FSYS_FILES_MAX 256
# define FSYS_QSLOTS 512
# define FSYS_CHUNK 65536
# define FSYS_DIRENTS_MAX 64

typedef struct s_fqent
{
	uint32_t	pid;
	uint32_t	n;
}	t_fqent;

typedef struct s_fquota
{
	t_spinlock	lock;
	uint32_t	ready;
	t_fqent		e[FSYS_QSLOTS];
}	t_fquota;

typedef struct s_fsio
{
	t_vfile		*f;
	t_uptr		ubuf;
	uint64_t	len;
	uint64_t	off;
	uint32_t	write;
	uint32_t	use_pos;
}	t_fsio;

typedef struct s_fsysent
{
	uint32_t	num;
	t_sysfn		fn;
	const char	*name;
}	t_fsysent;

void		fsys_quota_init(void);
int			fsys_quota_take(uint32_t pid);
void		fsys_quota_drop(uint32_t pid);
int			fsys_path_in(t_uptr p, uint64_t len, char *out);
int			fsys_writer(t_process **p);
int			fsys_get(uint64_t h, uint32_t type, uint32_t right, t_hget *hg);
int			fsys_get_any(uint64_t h, t_hget *hg);
int			fsys_handle_new(t_process *p, t_vfile *f, t_handle *out);
int64_t		fsys_rw(const t_sysargs *a, uint32_t write, uint32_t use_pos);
int64_t		sys_open(const t_sysargs *a);
int64_t		sys_read(const t_sysargs *a);
int64_t		sys_write(const t_sysargs *a);
int64_t		sys_pread(const t_sysargs *a);
int64_t		sys_pwrite(const t_sysargs *a);
int64_t		sys_seek(const t_sysargs *a);
int64_t		sys_stat(const t_sysargs *a);
int64_t		sys_fstat(const t_sysargs *a);
int64_t		sys_readdir(const t_sysargs *a);
int64_t		sys_mkdir(const t_sysargs *a);
int64_t		sys_unlink(const t_sysargs *a);
int64_t		sys_rename(const t_sysargs *a);
int64_t		sys_fsync(const t_sysargs *a);
int64_t		sys_truncate(const t_sysargs *a);

#endif
