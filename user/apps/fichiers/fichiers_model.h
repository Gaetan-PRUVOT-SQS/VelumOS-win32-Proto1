#ifndef FICHIERS_MODEL_H
# define FICHIERS_MODEL_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_types.h"

# define FF_ENTRIES_MAX 512
# define FF_SIZE_TEXT_MAX 24
# define FF_PROPS_MAX 200
# define FF_STAT_RO 0x1
# define FF_DIRENTS_CHUNK 8

typedef struct s_ffentry
{
	char		name[VFS_NAME_MAX + 1];
	uint32_t	is_dir;
}	t_ffentry;

typedef struct s_ffdir
{
	t_ffentry	items[FF_ENTRIES_MAX];
	uint32_t	count;
	uint32_t	truncated;
}	t_ffdir;

typedef struct s_ffops
{
	void	*ctx;
	int		(*list)(void *ctx, const char *path, t_ffdir *out);
	int		(*stat)(void *ctx, const char *path, t_vstat *out);
	int		(*mkdir)(void *ctx, const char *path);
	int		(*create)(void *ctx, const char *path);
	int		(*rename)(void *ctx, const char *from, const char *to);
	int		(*remove)(void *ctx, const char *path);
}	t_ffops;

int		ff_name_valid(const char *name);
int		ff_path_join(char *out, size_t cap, const char *dir, const char *name);
int		ff_path_parent(char *path);
int		ff_path_is_root(const char *path);
void	ff_sort(t_ffdir *d);
int		ff_size_text(char *out, size_t cap, uint64_t size);
int		ff_load(const t_ffops *ops, const char *path, t_ffdir *out);
int		ff_make_dir(const t_ffops *ops, const char *dir, const char *name);
int		ff_make_file(const t_ffops *ops, const char *dir, const char *name);
int		ff_rename(const t_ffops *ops, const char *dir, const char *from,
			const char *to);
int		ff_remove(const t_ffops *ops, const char *dir, const char *name);
int		ff_props(const t_ffops *ops, const char *dir, const char *name,
			char *out, size_t cap);
int		ff_count_text(char *out, size_t cap, uint32_t count);
const char	*ff_dir_title(const char *path);
const char	*ff_error_text(int code);
void	ff_ops_vfs(t_ffops *ops);

#endif
