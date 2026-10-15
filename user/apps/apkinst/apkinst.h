#ifndef APKINST_H
# define APKINST_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/pm.h"

# define APKINST_SYS_ROOT "/system/apps"
# define APKINST_DATA_ROOT "/data/apps"
# define APKINST_LINE_MAX 384

const char	*apkinst_reason(int err, int apk_reason_code);
int			apkinst_install(const char *path, t_pmentry *out);
void		apkinst_report(int err, const t_pmentry *e);

#endif
