#ifndef VFS_H
# define VFS_H

# include <stddef.h>
# include <stdint.h>
# include "abi/abi_syscall.h"
# include "abi/abi_types.h"
# include "block.h"

# define VFS_MOUNT_RO 0x1

struct	s_vfile;

int		vfs_boot_init(void);
int		vfs_mount(const char *target, const char *fs, t_blkdev *d, uint32_t fl);
int		vfs_umount(const char *target);
int		vfs_open(const char *path, uint32_t flags, struct s_vfile **out);
int		vfs_close(struct s_vfile *f);
int64_t	vfs_read(struct s_vfile *f, void *buf, size_t n);
int64_t	vfs_write(struct s_vfile *f, const void *buf, size_t n);
int64_t	vfs_pread(struct s_vfile *f, void *buf, size_t n, uint64_t off);
int64_t	vfs_pwrite(struct s_vfile *f, const void *b, size_t n, uint64_t off);
int64_t	vfs_seek(struct s_vfile *f, int64_t off, int whence);
int		vfs_stat(const char *path, t_vstat *out);
int		vfs_fstat(struct s_vfile *f, t_vstat *out);
int		vfs_readdir(struct s_vfile *f, t_dirent *out);
int		vfs_mkdir(const char *path, uint32_t mode);
int		vfs_unlink(const char *path);
int		vfs_rename(const char *from, const char *to);
int		vfs_fsync(struct s_vfile *f);
int		vfs_truncate(struct s_vfile *f, uint64_t size);
int		vfs_read_all(const char *path, void **buf, size_t *size);

# define VFS_STAT_RO 0x1

int		vfs_selftest(void);
int		vfs_mount_initrd(const char *target, const void *base, uint64_t sz);

#endif
