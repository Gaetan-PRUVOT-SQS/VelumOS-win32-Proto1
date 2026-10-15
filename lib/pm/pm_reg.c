#include "pm_int.h"

int	pm_registry_parse(t_span text, t_pmentry *out, uint32_t max)
{
	t_pmreg	r;

	r.e = out;
	r.max = max;
	r.n = 0;
	r.rejected = 0;
	return (pm_registry_scan(text, &r));
}

int	pm_index(const t_pmentry *e, uint32_t n, const char *package)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (pm_same_pkg(e[i].info.package, package))
			return ((int)i);
		i++;
	}
	return (E_NOENT);
}

int	pm_path(char *dst, const char *root, const char *name, const char *suf)
{
	size_t	r;
	size_t	n;
	size_t	s;

	r = pm_nlen(root, PM_ROOT_MAX);
	n = pm_nlen(name, PM_PATH_MAX);
	s = pm_nlen(suf, PM_PATH_MAX);
	if (r == 0 || r >= PM_ROOT_MAX || r + n + s + 2 > PM_PATH_MAX)
		return (E_RANGE);
	memcpy(dst, root, r);
	dst[r] = '/';
	memcpy(dst + r + 1, name, n);
	memcpy(dst + r + 1 + n, suf, s);
	dst[r + 1 + n + s] = '\0';
	return (0);
}

void	pm_defaults(t_pm *pm)
{
	pm->max_packages = PM_MAX_PACKAGES;
	pm->max_apk = PM_APK_MAX;
	pm->max_registry = PM_REG_MAX;
}
