#include "vfs_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static size_t	comp_len(const char *s)
{
	size_t	n;

	n = 0;
	while (s[n] && s[n] != '/')
		n++;
	return (n);
}

static void	pop_comp(char *out, size_t *o)
{
	while (*o > 0 && out[*o - 1] != '/')
		(*o)--;
	if (*o > 0)
		(*o)--;
}

static void	apply_comp(const char *c, size_t n, char *out, size_t *o)
{
	if (n == 1 && c[0] == '.')
		return ;
	if (n == 2 && c[0] == '.' && c[1] == '.')
	{
		pop_comp(out, o);
		return ;
	}
	out[(*o)++] = '/';
	memcpy(out + *o, c, n);
	*o += n;
}

static int	path_check(const char *in, size_t *len)
{
	*len = strnlen(in, VFS_PATH_MAX);
	if (*len >= VFS_PATH_MAX)
		return (E_RANGE);
	if (*len == 0 || in[0] != '/')
		return (E_INVAL);
	return (vpath_utf8_ok((const uint8_t *)in, *len));
}

int	vpath_norm(const char *in, char *out)
{
	size_t	len;
	size_t	i;
	size_t	o;
	size_t	n;
	int		rc;

	rc = path_check(in, &len);
	if (rc < 0)
		return (rc);
	i = 0;
	o = 0;
	while (i < len)
	{
		while (in[i] == '/')
			i++;
		n = comp_len(in + i);
		if (n)
			apply_comp(in + i, n, out, &o);
		i += n;
	}
	if (o == 0)
		out[o++] = '/';
	out[o] = '\0';
	return (0);
}
