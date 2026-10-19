#include "velum/err.h"
#include "velum/libk.h"
#include "../common/tbuf.h"
#include "fichiers_model.h"

static int	fold(int c)
{
	if (c >= 'A' && c <= 'Z')
		return (c + 'a' - 'A');
	return (c);
}

static int	name_cmp(const char *a, const char *b)
{
	size_t	i;

	i = 0;
	while (a[i] && fold((unsigned char)a[i]) == fold((unsigned char)b[i]))
		i++;
	if (fold((unsigned char)a[i]) != fold((unsigned char)b[i]))
		return (fold((unsigned char)a[i]) - fold((unsigned char)b[i]));
	return (strcmp(a, b));
}

static int	before(const t_ffentry *a, const t_ffentry *b)
{
	if (a->is_dir != b->is_dir)
		return (a->is_dir > b->is_dir);
	return (name_cmp(a->name, b->name) < 0);
}

void	ff_sort(t_ffdir *d)
{
	t_ffentry	moved;
	uint32_t	i;
	uint32_t	j;

	i = 1;
	while (i < d->count)
	{
		moved = d->items[i];
		j = i;
		while (j > 0 && before(&moved, &d->items[j - 1]))
		{
			d->items[j] = d->items[j - 1];
			j--;
		}
		d->items[j] = moved;
		i++;
	}
}

static int	skip_dots(t_ffdir *d)
{
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < d->count)
	{
		if (strcmp(d->items[i].name, ".") && strcmp(d->items[i].name, ".."))
		{
			if (n != i)
				d->items[n] = d->items[i];
			n++;
		}
		i++;
	}
	d->count = n;
	return (0);
}

int	ff_load(const t_ffops *ops, const char *path, t_ffdir *out)
{
	int	r;

	out->count = 0;
	out->truncated = 0;
	if (path[0] != '/')
		return (E_INVAL);
	r = ops->list(ops->ctx, path, out);
	if (r < 0)
		return (r);
	skip_dots(out);
	ff_sort(out);
	return (0);
}

static int	exists(const t_ffops *ops, const char *path)
{
	t_vstat	st;

	return (ops->stat(ops->ctx, path, &st) == 0);
}

static int	creation(const t_ffops *ops, const char *dir, const char *name,
				int dirflag)
{
	char	path[VFS_PATH_MAX];
	int		r;

	r = ff_path_join(path, sizeof(path), dir, name);
	if (r < 0)
		return (r);
	if (exists(ops, path))
		return (E_EXIST);
	if (dirflag)
		return (ops->mkdir(ops->ctx, path));
	return (ops->create(ops->ctx, path));
}

int	ff_make_dir(const t_ffops *ops, const char *dir, const char *name)
{
	return (creation(ops, dir, name, 1));
}

int	ff_make_file(const t_ffops *ops, const char *dir, const char *name)
{
	return (creation(ops, dir, name, 0));
}

int	ff_rename(const t_ffops *ops, const char *dir, const char *from,
		const char *to)
{
	char	src[VFS_PATH_MAX];
	char	dst[VFS_PATH_MAX];
	int		r;

	r = ff_path_join(src, sizeof(src), dir, from);
	if (r < 0)
		return (r);
	r = ff_path_join(dst, sizeof(dst), dir, to);
	if (r < 0)
		return (r);
	if (!strcmp(from, to))
		return (0);
	if (!exists(ops, src))
		return (E_NOENT);
	if (exists(ops, dst))
		return (E_EXIST);
	return (ops->rename(ops->ctx, src, dst));
}

int	ff_remove(const t_ffops *ops, const char *dir, const char *name)
{
	char	path[VFS_PATH_MAX];
	int		r;

	r = ff_path_join(path, sizeof(path), dir, name);
	if (r < 0)
		return (r);
	return (ops->remove(ops->ctx, path));
}

int	ff_props(const t_ffops *ops, const char *dir, const char *name,
		char *out, size_t cap)
{
	char	path[VFS_PATH_MAX];
	char	size[FF_SIZE_TEXT_MAX];
	t_vstat	st;
	t_tbuf	b;
	int		is_dir;
	int		r;

	r = ff_path_join(path, sizeof(path), dir, name);
	if (r < 0)
		return (r);
	r = ops->stat(ops->ctx, path, &st);
	if (r < 0)
		return (r);
	is_dir = (st.mode & S_TYPE_MASK) == S_TYPE_DIR;
	tb_init(&b, out, cap);
	tb_str(&b, name);
	tb_str(&b, is_dir ? " : dossier" : " : fichier");
	if (!is_dir)
	{
		ff_size_text(size, sizeof(size), st.size);
		tb_str(&b, ", ");
		tb_str(&b, size);
	}
	if (st.flags & FF_STAT_RO)
		tb_str(&b, ", lecture seule");
	return (tb_result(&b));
}
