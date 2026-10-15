#ifndef VFS_INT_H
# define VFS_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/vfs.h"
# include "velum/sync.h"

# define VFS_MOUNTS_MAX 8
# define VFS_IO_MAX 0x7ffff000ull
# define VFS_OFF_MAX 0x7fffffffffffffffull
# define VFS_READALL_MAX 0x4000000ull
# define VFS_NOLOC 0xffffffffffffffffull
# define VFS_ACC_MASK 0x3
# define VFS_O_KNOWN 0x106c3

typedef struct s_vio
{
	uint8_t			*dst;
	const uint8_t	*src;
	uint64_t		len;
	uint64_t		off;
}	t_vio;

typedef struct s_vnode
{
	uint64_t		ino;
	uint64_t		size;
	uint64_t		mtime_ns;
	uint64_t		ctime_ns;
	uint64_t		loc;
	uint32_t		first;
	uint32_t		mode;
	uint32_t		attr;
	uint32_t		refs;
	uint32_t		cur_idx;
	uint32_t		cur_clus;
	struct s_vnode	*next;
}	t_vnode;

typedef struct s_fsops
{
	const char	*name;
	int			(*lookup)(void *fs, const char *rel, t_vnode *out);
	int			(*open)(void *fs, t_vnode *n);
	int64_t		(*read)(void *fs, t_vnode *n, t_vio *io);
	int64_t		(*write)(void *fs, t_vnode *n, t_vio *io);
	int			(*readdir)(void *fs, t_vnode *d, uint64_t *ck, t_dirent *o);
	int			(*create)(void *fs, const char *rel, uint32_t t, t_vnode *o);
	int			(*remove)(void *fs, const char *rel);
	int			(*rename)(void *fs, const char *from, const char *to);
	int			(*truncate)(void *fs, t_vnode *n, uint64_t size);
	int			(*sync)(void *fs);
	void		(*release)(void *fs);
}	t_fsops;

typedef struct s_vmount
{
	char			path[VFS_PATH_MAX];
	uint32_t		len;
	uint32_t		flags;
	uint32_t		users;
	uint32_t		used;
	const t_fsops	*ops;
	void			*fs;
	t_vnode			*open;
	t_mutex			lock;
}	t_vmount;

typedef struct s_vfs
{
	t_vmount	mounts[VFS_MOUNTS_MAX];
	t_mutex		lock;
	uint32_t	ready;
}	t_vfs;

typedef struct s_vres
{
	char		norm[VFS_PATH_MAX];
	t_vmount	*mnt;
	const char	*rel;
}	t_vres;

struct s_vfile
{
	t_vmount	*mnt;
	t_vnode		*node;
	uint64_t	pos;
	uint32_t	flags;
	uint32_t	owner;
	t_mutex		lock;
};

typedef struct s_vfile	t_vfile;

t_vfs		*vfs_g(void);
void		vfs_state_init(void);
int			vpath_norm(const char *in, char *out);
int			vpath_utf8_ok(const uint8_t *s, size_t n);
const char	*vpath_next(const char *p, char *comp);
int			vpath_split(const char *rel, char *parent, const char **name);
int			vname_eq(const char *a, const char *b);
int			vfs_resolve(const char *path, t_vres *res);
void		vfs_mnt_put(t_vmount *m);
int			vfs_mount_add(const char *p, const t_fsops *o, void *s, uint32_t f);
int			vnode_get(t_vmount *m, const t_vnode *tmpl, t_vnode **out);
void		vnode_put(t_vmount *m, t_vnode *n);
int			vnode_busy(t_vmount *m, uint64_t ino);
void		vfs_fill_stat(const t_vmount *m, const t_vnode *n, t_vstat *o);
int			vfs_flags_ok(uint32_t flags);
int			vfs_ns_begin(const char *path, t_vres *r);
int			vfs_open_locked(t_vres *r, uint32_t flags, t_vnode **out);
int64_t		vfs_xfer(t_vfile *f, t_vio *io, int write, int use_pos);
int			vfs_initrd_mount(const char *t, const void *b, uint64_t sz);
int			vfs_fat_mount(const char *norm, t_blkdev *d, uint32_t fl);
int			vfs_initrd_boot(const char *norm);
void		vfs_sys_register(void);
uint64_t	vfs_now_ns(void);
void		*vfs_alloc(size_t size);

#endif
