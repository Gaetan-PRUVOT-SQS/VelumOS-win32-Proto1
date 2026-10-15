#include "fake.h"

static int64_t	f_read(void *ctx, const char *path, t_text out)
{
	t_ffile	*f;

	(void)ctx;
	if (fake_fs_tick() < 0)
		return (E_IO);
	f = fake_fs_get(path);
	if (!f)
		return (E_NOENT);
	if (f->len > out.cap)
		return (E_OVERFLOW);
	memcpy(out.p, f->data, f->len);
	return ((int64_t)f->len);
}

static int	f_write_new(void *ctx, const char *path, t_span data)
{
	(void)ctx;
	if (fake_fs_tick() < 0)
	{
		(void)fake_fs_put(path, data.p, data.len / 2);
		return (E_IO);
	}
	return (fake_fs_put(path, data.p, data.len));
}

static int	f_rename(void *ctx, const char *from, const char *to)
{
	t_ffile	*src;
	t_ffile	*dst;

	(void)ctx;
	if (fake_fs_tick() < 0)
		return (E_IO);
	src = fake_fs_get(from);
	if (!src)
		return (E_NOENT);
	dst = fake_fs_get(to);
	if (dst)
		dst->used = 0;
	strlcpy(src->path, to, PM_PATH_MAX);
	return (0);
}

static int	f_unlink(void *ctx, const char *path)
{
	t_ffile	*f;

	(void)ctx;
	if (fake_fs_tick() < 0)
		return (E_IO);
	f = fake_fs_get(path);
	if (!f)
		return (E_NOENT);
	f->used = 0;
	return (0);
}

const t_pmfs	*fake_fs_table(void)
{
	static t_pmfs	table;

	table.read = f_read;
	table.write_new = f_write_new;
	table.rename = f_rename;
	table.unlink = f_unlink;
	table.mkdir = NULL;
	return (&table);
}
