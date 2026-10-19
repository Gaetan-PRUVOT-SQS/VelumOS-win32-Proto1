#include "velum/err.h"
#include "velum/libk.h"
#include "fichiers_model.h"

int	ff_name_valid(const char *name)
{
	size_t	i;

	if (!name || !name[0] || strlen(name) > VFS_NAME_MAX)
		return (0);
	if (!strcmp(name, ".") || !strcmp(name, ".."))
		return (0);
	i = 0;
	while (name[i])
	{
		if (name[i] == '/' || (unsigned char)name[i] < 0x20
			|| name[i] == 0x7f)
			return (0);
		i++;
	}
	return (1);
}

int	ff_path_is_root(const char *path)
{
	return (path[0] == '/' && path[1] == '\0');
}

int	ff_path_join(char *out, size_t cap, const char *dir, const char *name)
{
	size_t	n;
	size_t	m;

	if (!ff_name_valid(name) || dir[0] != '/')
		return (E_INVAL);
	n = strlen(dir);
	m = strlen(name);
	if (ff_path_is_root(dir))
		n = 0;
	if (n + 1 + m + 1 > cap || n + 1 + m >= VFS_PATH_MAX)
		return (E_RANGE);
	memcpy(out, dir, n);
	out[n] = '/';
	memcpy(out + n + 1, name, m + 1);
	return (0);
}

int	ff_path_parent(char *path)
{
	size_t	n;

	if (path[0] != '/' || ff_path_is_root(path))
		return (E_INVAL);
	n = strlen(path);
	while (n > 1 && path[n - 1] == '/')
		n--;
	while (n > 1 && path[n - 1] != '/')
		n--;
	while (n > 1 && path[n - 1] == '/')
		n--;
	path[n] = '\0';
	if (n == 0)
		strlcpy(path, "/", 2);
	return (0);
}
