#include "velum/err.h"
#include "velum/libk.h"
#include "apkrun_trust.h"

int	apkrun_signer_check(t_pm *pm, const char *package, const uint8_t *cert)
{
	t_pmentry	known;
	int			err;

	if (!pm || !package || !cert)
		return (E_INVAL);
	err = pm_find(pm, package, &known);
	if (err == E_NOENT)
		return (E_ACCES);
	if (err < 0)
		return (err);
	if (memcmp(known.info.cert, cert, PM_CERT_LEN) != 0)
		return (E_ACCES);
	return (0);
}
