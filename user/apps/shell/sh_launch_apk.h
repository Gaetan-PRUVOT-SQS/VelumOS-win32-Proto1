#ifndef SH_LAUNCH_APK_H
# define SH_LAUNCH_APK_H

# include <stddef.h>
# include <stdint.h>
# include "velum/luna_launcher.h"

# define SH_LAUNCH_APKS_MAX 32
# define SH_LAUNCH_PKG_MAX 96
# define SH_LAUNCH_ORIGIN_SYSTEM 0
# define SH_LAUNCH_ORIGIN_DATA 1
# define SH_LAUNCH_REG_SYSTEM "/system/apps/paquets"
# define SH_LAUNCH_REG_DATA "/data/apps/paquets"
# define SH_LAUNCH_ROOT_SYSTEM "/system/apps"
# define SH_LAUNCH_ROOT_DATA "/data/apps"
# define SH_LAUNCH_REG_BUF 8192
# define SH_LID_APK_BASE 100
# define SH_LID_RUN 1000

typedef struct s_launchapk
{
	char		package[SH_LAUNCH_PKG_MAX];
	char		label[LUNA_LM_LABEL_MAX];
	uint32_t	origin;
}	t_launchapk;

typedef struct s_launchapks
{
	uint32_t	count;
	t_launchapk	list[SH_LAUNCH_APKS_MAX];
}	t_launchapks;

int	sh_launch_apk_parse(const char *buf, size_t len, uint32_t origin,
		t_launchapks *out);
int	sh_launch_apk_path(const t_launchapk *a, char *path, size_t size);

#endif
