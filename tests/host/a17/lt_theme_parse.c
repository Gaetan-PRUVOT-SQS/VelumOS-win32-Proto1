#include <stdlib.h>
#include <string.h>
#include "lt_theme_parse.h"

bool	lth_parse_line(const char *line, char *name, size_t cap, uint32_t *v)
{
	const char	*p;
	size_t		n;
	char		*end;

	if (strncmp(line, "# define VT_", 12) != 0)
		return (false);
	p = line + 9;
	n = strcspn(p, " \t(");
	if (p[n] == '(' || n == 0 || n >= cap)
		return (false);
	memcpy(name, p, n);
	name[n] = '\0';
	p += n;
	p += strspn(p, " \t");
	if (*p < '0' || *p > '9')
		return (false);
	*v = (uint32_t)strtoul(p, &end, 0);
	return (end != p);
}

const t_lthv	*lth_find(const char *name)
{
	size_t	i;

	i = 0;
	while (g_lth_all[i].name != NULL)
	{
		if (strcmp(g_lth_all[i].name, name) == 0)
			return (&g_lth_all[i]);
		i++;
	}
	return (NULL);
}

uint32_t	lth_hex(const char *s)
{
	return (0xff000000U | (uint32_t)strtoul(s + 1, NULL, 16));
}

uint32_t	lth_pct(uint32_t pct, uint32_t rgb)
{
	return (((pct * 255U + 50U) / 100U) << 24) | rgb;
}

size_t	lth_count(void)
{
	size_t	n;

	n = 0;
	while (g_lth_all[n].name != NULL)
		n++;
	return (n);
}
