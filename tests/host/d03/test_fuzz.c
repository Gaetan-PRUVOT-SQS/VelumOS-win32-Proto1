#include <stdlib.h>
#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static void	resized(uint8_t *p, size_t n)
{
	t_dex	d;
	t_span	s;

	p[32] = (uint8_t)n;
	p[33] = (uint8_t)(n >> 8);
	p[34] = (uint8_t)(n >> 16);
	p[35] = (uint8_t)(n >> 24);
	fake_seal(p, n);
	s.p = p;
	s.len = n;
	if (dex_open(&d, s) == E_OK)
		fake_walk(&d);
}

static void	truncation(void)
{
	t_span	s;
	t_span	cut;
	uint8_t	*p;
	t_dex	d;

	s = fake_fixture();
	cut.len = 0;
	while (cut.len < s.len)
	{
		p = fake_copy(s, cut.len);
		cut.p = p;
		h_true(dex_open(&d, cut) != E_OK, "prefixe refuse");
		if (cut.len >= 0x70)
			resized(p, cut.len);
		free(p);
		cut.len++;
	}
}

static uint32_t	mutate_once(t_span s, uint32_t *seed)
{
	uint8_t		*p;
	t_dex		d;
	uint32_t	k;
	uint32_t	ok;

	p = fake_copy(s, s.len);
	k = 1 + fake_rand(seed) % 3;
	while (k > 0)
	{
		p[fake_rand(seed) % s.len] = (uint8_t)fake_rand(seed);
		k--;
	}
	fake_seal(p, s.len);
	s.p = p;
	ok = (dex_open(&d, s) == E_OK);
	if (ok)
		fake_walk(&d);
	free(p);
	return (ok);
}

static void	mutations(void)
{
	uint32_t	seed;
	uint32_t	i;
	uint32_t	ok;

	seed = 0x2545f491;
	i = 0;
	ok = 0;
	while (i < 20000)
	{
		ok += mutate_once(fake_fixture(), &seed);
		i++;
	}
	h_true(ok > 1000, "des mutants passent dex_open et sont parcourus");
	h_true(ok < 20000, "des mutants sont refuses");
}

int	main(void)
{
	h_begin("d03/fuzz");
	h_true(fake_fixture().len > 0x70, "fixture lue");
	h_run("troncature a chaque octet", truncation);
	h_run("20 000 mutations a graine fixe, somme recalculee", mutations);
	return (h_end());
}
