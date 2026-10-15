#include "rng_int.h"
#include "velum/random.h"

int	random_selftest(void)
{
	int	rc;

	rc = crypto_selftest();
	if (!rc)
		rc = rng_selftest();
	if (!rc)
		rc = sys_selftest();
	return (rc);
}
