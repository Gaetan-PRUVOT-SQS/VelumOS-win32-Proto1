#include "velum/libk.h"
#include "sh_launch_apk.h"

static int	pkg_char_ok(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9') || c == '.' || c == '_');
}

static int	pkg_ok(const char *s, size_t n)
{
	size_t	i;

	if (n == 0 || n >= SH_LAUNCH_PKG_MAX || s[0] == '.' || s[n - 1] == '.')
		return (0);
	i = 0;
	while (i < n)
	{
		if (!pkg_char_ok(s[i]))
			return (0);
		if (s[i] == '.' && i + 1 < n && s[i + 1] == '.')
			return (0);
		i++;
	}
	return (1);
}

static size_t	label_cut(const char *s, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if ((uint8_t)s[i] < 0x20 || s[i] == 0x7f)
			return (0);
		i++;
	}
	if (n >= LUNA_LM_LABEL_MAX)
	{
		n = LUNA_LM_LABEL_MAX - 1;
		while (n > 0 && ((uint8_t)s[n] & 0xc0) == 0x80)
			n--;
	}
	return (n);
}

static int	known(const t_launchapks *o, const char *pkg, size_t n)
{
	uint32_t	i;

	i = 0;
	while (i < o->count)
	{
		if (strlen(o->list[i].package) == n
			&& memcmp(o->list[i].package, pkg, n) == 0)
			return (1);
		i++;
	}
	return (0);
}

static void	take_line(const char *s, size_t n, uint32_t origin,
				t_launchapks *o)
{
	const char	*a;
	const char	*b;
	const char	*c;
	size_t		lab;

	a = memchr(s, ';', n);
	b = NULL;
	c = NULL;
	if (a != NULL)
		b = memchr(a + 1, ';', n - (size_t)(a + 1 - s));
	if (b != NULL)
		c = memchr(b + 1, ';', n - (size_t)(b + 1 - s));
	if (c == NULL || !pkg_ok(s, (size_t)(a - s)) || o->count >= SH_LAUNCH_APKS_MAX
		|| known(o, s, (size_t)(a - s)))
		return ;
	lab = label_cut(b + 1, (size_t)(c - b - 1));
	if (lab == 0)
		return ;
	memset(&o->list[o->count], 0, sizeof(o->list[0]));
	memcpy(o->list[o->count].package, s, (size_t)(a - s));
	memcpy(o->list[o->count].label, b + 1, lab);
	o->list[o->count].origin = origin;
	o->count++;
}

int	sh_launch_apk_parse(const char *buf, size_t len, uint32_t origin,
		t_launchapks *out)
{
	size_t	start;
	size_t	end;

	if (out == NULL || (buf == NULL && len > 0))
		return (-1);
	start = 0;
	while (start < len)
	{
		end = start;
		while (end < len && buf[end] != '\n')
			end++;
		if (end > start && buf[end - 1] == '\r')
			end--;
		if (end > start)
			take_line(buf + start, end - start, origin, out);
		while (end < len && buf[end] != '\n')
			end++;
		start = end + 1;
	}
	return ((int)out->count);
}

int	sh_launch_apk_path(const t_launchapk *a, char *path, size_t size)
{
	const char	*root;

	if (a == NULL || path == NULL || size == 0)
		return (-1);
	root = SH_LAUNCH_ROOT_DATA;
	if (a->origin == SH_LAUNCH_ORIGIN_SYSTEM)
		root = SH_LAUNCH_ROOT_SYSTEM;
	path[0] = '\0';
	if (strlcpy(path, root, size) >= size
		|| strlcat(path, "/", size) >= size
		|| strlcat(path, a->package, size) >= size
		|| strlcat(path, ".apk", size) >= size)
	{
		path[0] = '\0';
		return (-1);
	}
	return (0);
}
