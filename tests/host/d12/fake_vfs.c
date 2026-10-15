#include <string.h>
#include "velum/err.h"
#include "velum/vfile.h"
#include "fake.h"

int	v_stat(const char *path, t_vstat *out)
{
	int	i;

	if (fv_tick() < 0)
		return (E_IO);
	i = fv_find(path);
	if (i < 0)
		return (E_NOENT);
	memset(out, 0, sizeof(*out));
	out->size = g_fv.f[i].len;
	return (0);
}

int	v_rename(const char *old_path, const char *new_path)
{
	int	i;

	if (fv_tick() < 0)
		return (E_IO);
	i = fv_find(old_path);
	if (i < 0)
		return (E_NOENT);
	if (fv_find(new_path) >= 0)
		return (E_EXIST);
	if (strlen(new_path) >= FV_NAME)
		return (E_RANGE);
	memcpy(g_fv.f[i].name, new_path, strlen(new_path) + 1);
	return (0);
}

int	v_unlink(const char *path)
{
	int	i;

	if (fv_tick() < 0)
		return (E_IO);
	i = fv_find(path);
	if (i < 0)
		return (E_NOENT);
	g_fv.f[i].used = 0;
	return (0);
}

int	v_mkdir(const char *path, uint32_t mode)
{
	(void)path;
	(void)mode;
	if (fv_tick() < 0)
		return (E_IO);
	return (0);
}

int	v_fsync(t_handle file)
{
	(void)file;
	if (fv_tick() < 0)
		return (E_IO);
	return (0);
}
