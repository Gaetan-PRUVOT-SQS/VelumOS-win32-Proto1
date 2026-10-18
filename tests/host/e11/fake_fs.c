#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vfile.h"
#include "velum/vobj.h"
#include "fake_fs.h"

# define FEED_MAX (FF_ENTRIES_MAX + 4)

static int	find(const t_fakefs *fs, const char *path)
{
	uint32_t	i;

	i = 0;
	while (i < fs->count)
	{
		if (!strcmp(fs->nodes[i].path, path))
			return ((int)i);
		i++;
	}
	return (-1);
}

void	fake_init(t_fakefs *fs)
{
	memset(fs, 0, sizeof(*fs));
	fake_add(fs, "/", 1, 0);
}

void	fake_add(t_fakefs *fs, const char *path, uint32_t is_dir,
		uint64_t size)
{
	t_fakenode	*n;

	n = &fs->nodes[fs->count++];
	strlcpy(n->path, path, sizeof(n->path));
	n->is_dir = is_dir;
	n->size = size;
}

int	fake_has(const t_fakefs *fs, const char *path)
{
	return (find(fs, path) >= 0);
}

static int	is_child(const char *dir, const char *path, const char **name)
{
	size_t	n;

	n = strlen(dir);
	if (n == 1)
		n = 0;
	if (strncmp(path, dir, n) || path[n] != '/' || !path[n + 1])
		return (0);
	if (strchr(path + n + 1, '/'))
		return (0);
	*name = path + n + 1;
	return (1);
}

static int	op_list(void *ctx, const char *path, t_ffdir *out)
{
	t_fakefs	*fs;
	const char	*name;
	uint32_t	i;

	fs = ctx;
	if (fs->fail_list)
		return (fs->fail_list);
	if (find(fs, path) < 0)
		return (E_NOENT);
	i = 0;
	while (i < fs->count)
	{
		if (is_child(path, fs->nodes[i].path, &name)
			&& out->count < FF_ENTRIES_MAX)
		{
			strlcpy(out->items[out->count].name, name, VFS_NAME_MAX + 1);
			out->items[out->count++].is_dir = fs->nodes[i].is_dir;
		}
		i++;
	}
	return (0);
}

static int	op_stat(void *ctx, const char *path, t_vstat *out)
{
	t_fakefs	*fs;
	int			i;

	fs = ctx;
	i = find(fs, path);
	if (i < 0)
		return (E_NOENT);
	memset(out, 0, sizeof(*out));
	out->size = fs->nodes[i].size;
	out->mode = fs->nodes[i].is_dir ? S_TYPE_DIR | 0755 : S_TYPE_REG | 0644;
	out->flags = fs->nodes[i].ro;
	return (0);
}

static int	op_add(void *ctx, const char *path, uint32_t is_dir)
{
	t_fakefs	*fs;

	fs = ctx;
	if (fs->fail_write)
		return (fs->fail_write);
	fake_add(fs, path, is_dir, 0);
	fs->writes++;
	return (0);
}

static int	op_mkdir(void *ctx, const char *path)
{
	return (op_add(ctx, path, 1));
}

static int	op_create(void *ctx, const char *path)
{
	return (op_add(ctx, path, 0));
}

static int	op_rename(void *ctx, const char *from, const char *to)
{
	t_fakefs	*fs;

	fs = ctx;
	if (fs->fail_write)
		return (fs->fail_write);
	strlcpy(fs->nodes[find(fs, from)].path, to, VFS_PATH_MAX);
	fs->writes++;
	return (0);
}

static int	op_remove(void *ctx, const char *path)
{
	t_fakefs	*fs;
	const char	*name;
	uint32_t	i;
	int			at;

	fs = ctx;
	at = find(fs, path);
	if (fs->fail_write)
		return (fs->fail_write);
	if (at < 0)
		return (E_NOENT);
	i = 0;
	while (i < fs->count)
		if (is_child(path, fs->nodes[i++].path, &name))
			return (E_NOTEMPTY);
	fs->nodes[at] = fs->nodes[--fs->count];
	fs->writes++;
	return (0);
}

void	fake_ops(t_ffops *ops, t_fakefs *fs)
{
	ops->ctx = fs;
	ops->list = op_list;
	ops->stat = op_stat;
	ops->mkdir = op_mkdir;
	ops->create = op_create;
	ops->rename = op_rename;
	ops->remove = op_remove;
}

static t_dirent	g_feed[FEED_MAX];
static uint32_t	g_feed_n;
static uint32_t	g_feed_pos;
static int		g_open_rc;

void	fake_vfs_reset(void)
{
	g_feed_n = 0;
	g_feed_pos = 0;
	g_open_rc = 0;
	memset(g_feed, 0, sizeof(g_feed));
}

void	fake_vfs_open_rc(int rc)
{
	g_open_rc = rc;
}

int	fake_vfs_push_raw(const char *bytes, uint32_t namelen, uint32_t type)
{
	t_dirent	*e;
	uint32_t	n;

	if (g_feed_n >= FEED_MAX)
		return (-1);
	e = &g_feed[g_feed_n];
	memset(e, 0, sizeof(*e));
	e->inode = g_feed_n + 1;
	e->type = type;
	e->namelen = namelen;
	n = namelen;
	if (n > sizeof(e->name))
		n = (uint32_t)sizeof(e->name);
	if (bytes && n > 0)
		memcpy(e->name, bytes, n);
	g_feed_n++;
	return (0);
}

int	fake_vfs_push(const char *name, uint32_t is_dir)
{
	if (!name)
		return (-1);
	return (fake_vfs_push_raw(name, (uint32_t)strlen(name),
			is_dir ? S_TYPE_DIR : S_TYPE_REG));
}

int64_t	v_open(const char *path, uint32_t flags, uint32_t mode)
{
	(void)mode;
	if (!path || path[0] != '/')
		return (E_INVAL);
	if (g_open_rc < 0)
		return (g_open_rc);
	if ((flags & O_DIRECTORY) == 0)
		return (E_NOTDIR);
	g_feed_pos = 0;
	return (1);
}

int	v_close(t_handle h)
{
	(void)h;
	return (0);
}

int	v_readdir(t_handle dir, t_dirent *out, uint32_t max)
{
	uint32_t	n;

	(void)dir;
	if (!out)
		return (E_FAULT);
	n = 0;
	while (n < max && g_feed_pos < g_feed_n)
	{
		out[n] = g_feed[g_feed_pos];
		g_feed_pos++;
		n++;
	}
	return ((int)n);
}

int	v_stat(const char *path, t_vstat *out)
{
	(void)path;
	(void)out;
	return (E_NOSYS);
}

int	v_mkdir(const char *path, uint32_t mode)
{
	(void)path;
	(void)mode;
	return (E_NOSYS);
}

int	v_unlink(const char *path)
{
	(void)path;
	return (E_NOSYS);
}

int	v_rename(const char *old_path, const char *new_path)
{
	(void)old_path;
	(void)new_path;
	return (E_NOSYS);
}
