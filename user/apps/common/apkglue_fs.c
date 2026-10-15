#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vfile.h"
#include "apkglue.h"

int	apkglue_old_path(char *dst, const char *path)
{
	size_t	n;

	n = strlen(path);
	if (n + sizeof(APKGLUE_OLD) > APKGLUE_PATH_MAX)
		return (E_RANGE);
	memcpy(dst, path, n);
	memcpy(dst + n, APKGLUE_OLD, sizeof(APKGLUE_OLD));
	return (0);
}

static int	fs_rename(void *ctx, const char *from, const char *to)
{
	char	old[APKGLUE_PATH_MAX];
	t_vstat	st;
	int		r;

	(void)ctx;
	if (apkglue_old_path(old, to) < 0)
		return (E_RANGE);
	r = v_stat(to, &st);
	if (r < 0 && r != E_NOENT)
		return (r);
	if (r == 0)
	{
		(void)v_unlink(old);
		r = v_rename(to, old);
		if (r < 0)
			return (r);
	}
	r = v_rename(from, to);
	if (r < 0 && v_stat(to, &st) == E_NOENT)
		(void)v_rename(old, to);
	if (r < 0)
		return (r);
	(void)v_unlink(old);
	return (0);
}

static int	fs_unlink(void *ctx, const char *path)
{
	char	old[APKGLUE_PATH_MAX];
	int		r;

	(void)ctx;
	r = v_unlink(path);
	if (apkglue_old_path(old, path) == 0)
		(void)v_unlink(old);
	return (r);
}

static int	fs_mkdir(void *ctx, const char *path)
{
	(void)ctx;
	return (v_mkdir(path, 0755));
}

const t_pmfs	*apkglue_fs(void)
{
	static const t_pmfs	fs = {apkglue_fs_read, apkglue_fs_write_new,
		fs_rename, fs_unlink, fs_mkdir};

	return (&fs);
}
