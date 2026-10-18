#ifndef APKRUN_TRUST_H
# define APKRUN_TRUST_H

# include <stdint.h>
# include "velum/apk/pm.h"

int	apkrun_signer_check(t_pm *pm, const char *package, const uint8_t *cert);

#endif
