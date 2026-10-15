#include <stdlib.h>
#include "harness.h"
#include "d01.h"

static void	troncature(void)
{
	t_blob	zip;
	t_zip	z;
	size_t	k;
	uint8_t	*in;

	zip = d01_load("ok.zip");
	k = 0;
	while (k < zip.len)
	{
		in = d01_dup(zip.p, k);
		h_true(zip_open(&z, d01_span(in, k)) < 0, "archive tronquee");
		free(in);
		k++;
	}
	free(zip.p);
}

static void	mutations(void)
{
	t_blob		zip;
	t_zip		z;
	uint8_t		*in;
	uint32_t	seed;
	int			i;

	zip = d01_load("ok.zip");
	seed = 0xd01d01;
	i = 0;
	while (i < 12000)
	{
		in = d01_dup(zip.p, zip.len);
		d01_mutate(in, zip.len, &seed);
		if (zip_open(&z, d01_span(in, zip.len)) == 0)
			h_eq_i64("parcours", d01_walk(&z), 0);
		else
			h_eq_i64("ferme", z.count, 0);
		free(in);
		i++;
	}
	free(zip.p);
}

int	main(void)
{
	h_begin("d01 zip fuzz");
	h_run("troncature_chaque_octet", troncature);
	h_run("mutations_graine_fixe", mutations);
	return (h_end());
}
