#ifndef PM_H
# define PM_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apkdef.h"

# define PM_PKG_MAX 128
# define PM_LABEL_MAX 64
# define PM_ACT_MAX 192
# define PM_CERT_LEN 32
# define PM_PATH_MAX 192
# define PM_ROOT_MAX 48
# define PM_MAX_PACKAGES 64
# define PM_APK_MAX 67108864
# define PM_REG_MAX 65536
# define PM_REG_NAME "paquets"
# define PM_ORIGIN_SYSTEM 0
# define PM_ORIGIN_DATA 1

typedef struct s_pminfo
{
	char		package[PM_PKG_MAX];
	uint32_t	version_code;
	char		label[PM_LABEL_MAX];
	char		activity[PM_ACT_MAX];
	uint8_t		cert[PM_CERT_LEN];
}	t_pminfo;

typedef struct s_pmentry
{
	t_pminfo	info;
	uint32_t	origin;
	char		path[PM_PATH_MAX];
}	t_pmentry;

typedef struct s_pmreg
{
	t_pmentry	*e;
	uint32_t	max;
	uint32_t	n;
	uint32_t	rejected;
}	t_pmreg;

typedef struct s_pmfs
{
	int64_t	(*read)(void *ctx, const char *path, t_text out);
	int		(*write_new)(void *ctx, const char *path, t_span data);
	int		(*rename)(void *ctx, const char *from, const char *to);
	int		(*unlink)(void *ctx, const char *path);
	int		(*mkdir)(void *ctx, const char *path);
}	t_pmfs;

typedef struct s_pm
{
	const char		*sys_root;
	const char		*data_root;
	const t_pmfs	*fs;
	int				(*inspect)(t_span apk, t_pminfo *out);
	void			*ctx;
	uint32_t		max_packages;
	uint32_t		max_apk;
	uint32_t		max_registry;
}	t_pm;

int		pm_install(t_pm *pm, t_span apk, t_pmentry *out);
int		pm_remove(t_pm *pm, const char *package);
int		pm_list(t_pm *pm, t_pmentry *out, uint32_t max);
int		pm_find(t_pm *pm, const char *package, t_pmentry *out);
int		pm_registry_parse(t_span text, t_pmentry *out, uint32_t max);
int		pm_registry_format(const t_pmentry *e, uint32_t n, t_text out);
int		pm_registry_scan(t_span text, t_pmreg *reg);
void	pm_defaults(t_pm *pm);

#endif
