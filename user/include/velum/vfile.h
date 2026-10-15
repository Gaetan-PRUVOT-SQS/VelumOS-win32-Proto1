#ifndef VFILE_H
# define VFILE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

int64_t	v_open(const char *path, uint32_t flags, uint32_t mode);
int64_t	v_read(t_handle file, void *buf, size_t len);
int64_t	v_write(t_handle file, const void *buf, size_t len);
int64_t	v_pread(t_handle file, void *buf, size_t len, uint64_t offset);
int64_t	v_pwrite(t_handle file, const void *buf, size_t len, uint64_t offset);
int64_t	v_seek(t_handle file, int64_t offset, int whence);
int		v_stat(const char *path, t_vstat *out);
int		v_fstat(t_handle file, t_vstat *out);
int		v_readdir(t_handle dir, t_dirent *out, uint32_t max);
int		v_mkdir(const char *path, uint32_t mode);
int		v_unlink(const char *path);
int		v_rename(const char *old_path, const char *new_path);
int		v_fsync(t_handle file);
int		v_truncate(t_handle file, uint64_t size);

#endif
