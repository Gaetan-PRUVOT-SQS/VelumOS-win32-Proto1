#include "vfs_int.h"
#include "velum/err.h"
#include "velum/libk.h"

const char	*vpath_next(const char *p, char *comp)
{
	size_t	n;

	while (*p == '/')
		p++;
	if (*p == '\0')
		return (NULL);
	n = 0;
	while (p[n] && p[n] != '/')
	{
		if (n < VFS_NAME_MAX)
			comp[n] = p[n];
		n++;
	}
	if (n > VFS_NAME_MAX)
		n = VFS_NAME_MAX;
	comp[n] = '\0';
	while (*p && *p != '/')
		p++;
	return (p);
}

int	vpath_split(const char *rel, char *parent, const char **name)
{
	const char	*slash;
	size_t		n;

	if (rel[0] == '\0')
		return (E_INVAL);
	slash = strrchr(rel, '/');
	if (slash == NULL)
	{
		parent[0] = '\0';
		*name = rel;
		return (0);
	}
	n = (size_t)(slash - rel);
	if (n >= VFS_PATH_MAX)
		return (E_RANGE);
	memcpy(parent, rel, n);
	parent[n] = '\0';
	*name = slash + 1;
	if (**name == '\0')
		return (E_INVAL);
	return (0);
}
