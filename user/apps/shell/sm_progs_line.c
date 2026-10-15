#include "velum/err.h"
#include "velum/libk.h"
#include "../common/utf8.h"
#include "sm_progs.h"

static int	name_ok(const char *s, size_t n)
{
	size_t	i;

	if (n == 0 || n >= SM_PROG_NAME_MAX || !utf8_valid(s, n))
		return (0);
	i = 0;
	while (i < n)
	{
		if ((uint8_t)s[i] < 0x20 || (uint8_t)s[i] == 0x7f)
			return (0);
		i++;
	}
	return (1);
}

static int	path_ok(const char *s, size_t n)
{
	size_t	i;

	if (n <= SM_PROG_ROOT_LEN || n >= SM_PROG_PATH_MAX
		|| memcmp(s, SM_PROG_ROOT, SM_PROG_ROOT_LEN) != 0 || s[n - 1] == '/')
		return (0);
	i = 0;
	while (i < n)
	{
		if ((uint8_t)s[i] <= 0x20 || (uint8_t)s[i] >= 0x7f || s[i] == ';')
			return (0);
		if (s[i] == '/' && i + 1 < n && (s[i + 1] == '/' || s[i + 1] == '.'))
			return (0);
		i++;
	}
	return (1);
}

static size_t	field_end(const char *s, size_t n, size_t from)
{
	while (from < n && s[from] != ';')
		from++;
	return (from);
}

int	sm_progs_line(t_smprog *p, const char *s, size_t n)
{
	size_t	e1;
	size_t	e2;

	e1 = field_end(s, n, 0);
	if (e1 == n)
		return (E_INVAL);
	e2 = field_end(s, n, e1 + 1);
	if (!name_ok(s, e1) || !path_ok(s + e1 + 1, e2 - e1 - 1))
		return (E_INVAL);
	if (e2 < n && !path_ok(s + e2 + 1, n - e2 - 1))
		return (E_INVAL);
	memset(p, 0, sizeof(*p));
	memcpy(p->name, s, e1);
	memcpy(p->path, s + e1 + 1, e2 - e1 - 1);
	if (e2 < n)
		memcpy(p->arg, s + e2 + 1, n - e2 - 1);
	return (0);
}
