#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vfile.h"
#include "velum/vobj.h"
#include "fichiers_model.h"

static int	add_entry(t_ffdir *out, const t_dirent *e)
{
	t_ffentry	*it;

	if (out->count >= FF_ENTRIES_MAX)
	{
		out->truncated = 1;
		return (0);
	}
	if (e->namelen == 0 || e->namelen > VFS_NAME_MAX)
		return (0);
	it = &out->items[out->count];
	memcpy(it->name, e->name, e->namelen);
	it->name[e->namelen] = '\0';
	it->is_dir = (e->type & S_TYPE_MASK) == S_TYPE_DIR;
	out->count++;
	return (1);
}

static int	drain(t_handle dir, t_ffdir *out)
{
	t_dirent	chunk[FF_DIRENTS_CHUNK];
	int			n;
	int			i;

	n = v_readdir(dir, chunk, FF_DIRENTS_CHUNK);
	while (n > 0)
	{
		i = 0;
		while (i < n)
		{
			add_entry(out, &chunk[i]);
			i++;
		}
		if (out->truncated)
			return (0);
		n = v_readdir(dir, chunk, FF_DIRENTS_CHUNK);
	}
	return (n);
}

static int	vfs_list(void *ctx, const char *path, t_ffdir *out)
{
	int64_t	h;
	int		r;

	(void)ctx;
	h = v_open(path, O_RDONLY | O_DIRECTORY, 0);
	if (h < 0)
		return ((int)h);
	r = drain((t_handle)h, out);
	v_close((t_handle)h);
	return (r);
}

static int	vfs_stat_op(void *ctx, const char *path, t_vstat *out)
{
	(void)ctx;
	return (v_stat(path, out));
}

static int	vfs_mkdir_op(void *ctx, const char *path)
{
	(void)ctx;
	return (v_mkdir(path, 0755));
}

static int	vfs_create_op(void *ctx, const char *path)
{
	int64_t	h;

	(void)ctx;
	h = v_open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (h < 0)
		return ((int)h);
	v_close((t_handle)h);
	return (0);
}

static int	vfs_rename_op(void *ctx, const char *from, const char *to)
{
	(void)ctx;
	return (v_rename(from, to));
}

static int	vfs_remove_op(void *ctx, const char *path)
{
	(void)ctx;
	return (v_unlink(path));
}

void	ff_ops_vfs(t_ffops *ops)
{
	ops->ctx = NULL;
	ops->list = vfs_list;
	ops->stat = vfs_stat_op;
	ops->mkdir = vfs_mkdir_op;
	ops->create = vfs_create_op;
	ops->rename = vfs_rename_op;
	ops->remove = vfs_remove_op;
}
