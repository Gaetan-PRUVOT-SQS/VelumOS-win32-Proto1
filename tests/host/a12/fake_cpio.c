#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fake.h"

uint8_t	*fake_dup(const void *p, size_t n)
{
	uint8_t	*d;

	d = malloc(n + (n == 0));
	if (d == NULL)
		abort();
	if (n)
		memcpy(d, p, n);
	return (d);
}

static int	add_long(t_vbuf *b)
{
	char	name[201];

	memset(name, 'L', 200);
	name[0] = 'l';
	name[200] = '\0';
	return (vst_cpio_add(b, name, 0100644, "long"));
}

int	fakecpio_valid(t_vbuf *b)
{
	int	rc;

	rc = vst_cpio_add(b, ".", 040755, NULL);
	if (rc == 0)
		rc = vst_cpio_add(b, "system", 040755, NULL);
	if (rc == 0)
		rc = vst_cpio_add(b, "system/bin/hello", 0100755, "bonjour\n");
	if (rc == 0)
		rc = vst_cpio_add(b, "./vide", 0100644, NULL);
	if (rc == 0)
		rc = vst_cpio_add(b, "x/y/z", 0100644, "zz");
	if (rc == 0)
		rc = add_long(b);
	if (rc == 0)
		rc = vst_cpio_add(b, "system/bin/hello", 0100755, "nouveau");
	if (rc == 0)
		rc = vst_cpio_add(b, "lien", 0120777, "system");
	if (rc == 0)
		rc = vst_cpio_add(b, "TRAILER!!!", 0, NULL);
	return (rc);
}

uint64_t	fake_seed(void)
{
	uint64_t	seed;
	char		*env;

	env = getenv("A12_SEED");
	seed = (uint64_t)time(NULL);
	if (env)
		seed = strtoull(env, NULL, 0);
	if (seed == 0)
		seed = 1;
	printf("graine %llu (A12_SEED pour rejouer)\n", (unsigned long long)seed);
	return (seed);
}
