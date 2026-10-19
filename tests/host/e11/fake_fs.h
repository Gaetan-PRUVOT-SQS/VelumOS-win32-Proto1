#ifndef FAKE_FS_H
# define FAKE_FS_H

# include "fichiers_model.h"

# define FAKE_MAX 64

typedef struct s_fakenode
{
	char		path[VFS_PATH_MAX];
	uint32_t	is_dir;
	uint64_t	size;
	uint32_t	ro;
}	t_fakenode;

typedef struct s_fakefs
{
	t_fakenode	nodes[FAKE_MAX];
	uint32_t	count;
	int			fail_list;
	int			fail_write;
	uint32_t	writes;
}	t_fakefs;

void	fake_init(t_fakefs *fs);
void	fake_add(t_fakefs *fs, const char *path, uint32_t is_dir,
			uint64_t size);
void	fake_ops(t_ffops *ops, t_fakefs *fs);
int		fake_has(const t_fakefs *fs, const char *path);
void	fake_vfs_reset(void);
void	fake_vfs_open_rc(int rc);
int		fake_vfs_push(const char *name, uint32_t is_dir);
int		fake_vfs_push_raw(const char *bytes, uint32_t namelen,
			uint32_t type);

#endif
