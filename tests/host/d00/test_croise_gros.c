#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "d00.h"
#include "velum/apk/apk.h"

#define MIO 1048576u

static int	verify(t_span file)
{
	t_apk		apk;
	t_apksig	sig;
	int			rc;

	rc = apk_open(&apk, file);
	if (rc == 0)
		rc = apk_verify(&apk, &sig);
	return (rc);
}

static int	verify_flipped(t_span file, size_t off)
{
	uint8_t	*copy;
	int		rc;

	copy = malloc(file.len);
	if (!copy)
		return (1);
	memcpy(copy, file.p, file.len);
	copy[off] ^= 0x01;
	rc = verify((t_span){copy, file.len});
	free(copy);
	return (rc);
}

static void	condensat_sur_plusieurs_blocs(void)
{
	t_span	file;

	file = d00_load("gros.apk");
	h_true(file.len > 2 * MIO, "APK de plus de 2 Mio");
	h_eq_i64("signature valide", verify(file), 0);
	h_true(verify_flipped(file, 4096) < 0, "octet du premier bloc");
	h_true(verify_flipped(file, MIO - 1) < 0, "dernier octet du bloc 1");
	h_true(verify_flipped(file, MIO) < 0, "premier octet du bloc 2");
	h_true(verify_flipped(file, MIO + MIO / 2) < 0, "milieu du bloc 2");
	h_true(verify_flipped(file, 2 * MIO) < 0, "premier octet du bloc 3");
	h_true(verify_flipped(file, 2 * MIO + 4321) < 0, "octet du bloc 3");
	d00_free(file);
}

int	main(void)
{
	h_begin("d00/croise-gros");
	h_run("condensat_sur_plusieurs_blocs", condensat_sur_plusieurs_blocs);
	return (h_end());
}
