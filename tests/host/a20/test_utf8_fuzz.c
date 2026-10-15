#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "utf8.h"

static char	random_byte(uint32_t *seed)
{
	uint32_t	r;

	r = next_rand(seed);
	if (r % 5 == 0)
		return ((char)(0x20 + (r >> 8) % 95));
	if (r % 5 == 1)
		return ((char)((r >> 8) % 33));
	if (r % 5 == 2)
		return ((char)(0xc2 + (r >> 8) % 51));
	if (r % 5 == 3)
		return ((char)(0x80 + (r >> 8) % 64));
	return ((char)(r >> 8));
}

static void	check_output(const char *out, size_t n, size_t size)
{
	size_t	i;
	int		plain;

	h_true(n < size, "tient dans le tampon");
	h_true(strlen(out) == n, "longueur rendue");
	h_true(utf8_valid(out, n), "sortie UTF-8 valide");
	i = 0;
	plain = 1;
	while (i < n)
	{
		if ((uint8_t)out[i] < 0x20 || out[i] == 0x7f)
			plain = 0;
		i++;
	}
	h_true(plain, "aucun caractere de controle");
}

static void	fuzz_once(uint32_t *seed)
{
	char	src[48];
	char	out[48];
	size_t	len;
	size_t	size;
	size_t	i;

	len = next_rand(seed) % 41;
	size = 1 + next_rand(seed) % 47;
	i = 0;
	while (i < len)
	{
		src[i] = random_byte(seed);
		i++;
	}
	check_output(out, utf8_clean_copy(out, size, src, len), size);
}

static void	fuzz_rounds(void)
{
	uint32_t	seed;
	uint32_t	i;

	seed = 0x1234abcd;
	i = 0;
	while (i < 20000)
	{
		fuzz_once(&seed);
		i++;
	}
}

int	main(void)
{
	h_begin("a20/utf8-fuzz");
	h_run("clean: fuzz 20000 tours, graine 0x1234abcd", fuzz_rounds);
	return (h_end());
}
