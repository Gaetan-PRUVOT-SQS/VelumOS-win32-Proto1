#include "pm_int.h"

static int	pkg_char(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9') || c == '_' || c == '.');
}

int	pm_pkg_ok(const char *s, size_t n)
{
	size_t	i;
	int		dots;

	if (n == 0 || n >= PM_PKG_MAX || s[0] == '.' || s[n - 1] == '.')
		return (0);
	i = 0;
	dots = 0;
	while (i < n)
	{
		if (!pkg_char(s[i]))
			return (0);
		if (s[i] == '.' && s[i + 1] == '.')
			return (0);
		dots += (s[i] == '.');
		i++;
	}
	return (dots > 0);
}

int	pm_label_ok(const char *s, size_t n)
{
	size_t	i;

	if (n >= PM_LABEL_MAX || !pm_utf8_ok(s, n))
		return (0);
	i = 0;
	while (i < n)
	{
		if ((uint8_t)s[i] < 0x20 || s[i] == 0x7f || s[i] == ';')
			return (0);
		i++;
	}
	return (1);
}

int	pm_act_ok(const char *s, size_t n)
{
	size_t	i;
	char	c;

	if (n < 3 || n >= PM_ACT_MAX || s[0] != 'L' || s[n - 1] != ';')
		return (0);
	i = 1;
	while (i < n - 1)
	{
		c = s[i];
		if (c == '.' || (!pkg_char(c) && c != '/' && c != '$'))
			return (0);
		i++;
	}
	return (1);
}

int	pm_info_ok(const t_pminfo *i)
{
	if (!pm_pkg_ok(i->package, pm_nlen(i->package, PM_PKG_MAX)))
		return (0);
	if (!pm_label_ok(i->label, pm_nlen(i->label, PM_LABEL_MAX)))
		return (0);
	return (pm_act_ok(i->activity, pm_nlen(i->activity, PM_ACT_MAX)));
}
