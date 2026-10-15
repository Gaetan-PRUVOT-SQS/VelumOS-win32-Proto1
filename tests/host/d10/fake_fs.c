#include "fake.h"

t_fakefs	g_fs;

void	fake_fs_reset(void)
{
	memset(&g_fs, 0, sizeof(g_fs));
}

int	fake_fs_tick(void)
{
	g_fs.ops++;
	if (g_fs.fail_at > 0 && (g_fs.ops == g_fs.fail_at
			|| (g_fs.crash && g_fs.ops > g_fs.fail_at)))
		return (E_IO);
	return (0);
}

t_ffile	*fake_fs_get(const char *path)
{
	int	i;

	i = 0;
	while (i < FAKE_FILES)
	{
		if (g_fs.f[i].used && strcmp(g_fs.f[i].path, path) == 0)
			return (&g_fs.f[i]);
		i++;
	}
	return (NULL);
}

int	fake_fs_put(const char *path, const void *data, size_t len)
{
	t_ffile	*f;
	int		i;

	f = fake_fs_get(path);
	i = 0;
	while (!f && i < FAKE_FILES && g_fs.f[i].used)
		i++;
	if ((!f && i == FAKE_FILES) || len > FAKE_DATA
		|| strlen(path) >= PM_PATH_MAX)
		return (E_NOSPC);
	if (!f)
		f = &g_fs.f[i];
	f->used = 1;
	strlcpy(f->path, path, PM_PATH_MAX);
	f->len = len;
	memcpy(f->data, data, len);
	return (0);
}
