#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"

static uint32_t	g_seed = 0x2545f491;

static uint32_t	rnd(void)
{
	g_seed ^= g_seed << 13;
	g_seed ^= g_seed >> 17;
	g_seed ^= g_seed << 5;
	return (g_seed);
}

static void	probe(const uint8_t *src, uint32_t units)
{
	t_dswitch	sw;
	t_darray	ar;
	t_dinsn		in;
	t_span		s;
	uint32_t	pc;

	s.len = 2 * (size_t)units;
	s.p = malloc(s.len + 1);
	if (!s.p)
		exit(2);
	memcpy((uint8_t *)s.p, src, s.len);
	pc = 0;
	while (pc <= units + 1)
	{
		dexcode_decode(s, pc, &in);
		dexcode_switch(s, pc, &sw);
		dexcode_array_data(s, pc, &ar);
		pc++;
	}
	free((uint8_t *)s.p);
}

static void	mutate_once(const t_dcodelimits *l)
{
	uint32_t	units;
	uint32_t	k;

	fake_sample((int)(rnd() % 4));
	units = g_c.n;
	k = 1 + rnd() % 3;
	while (k-- > 0)
		g_c.b[rnd() % (2 * units)] = (uint8_t)rnd();
	if (rnd() % 8 == 0)
		units = rnd() % (units + 1);
	fake_exact(g_c.b, units, l);
	probe(g_c.b, units);
}

static void	mutations(void)
{
	t_dcodelimits	l;
	int				i;

	l = fake_lim(3);
	g_c.accepted = 0;
	i = 0;
	while (i < 12000)
	{
		mutate_once(&l);
		i++;
	}
	h_eq_i64("mutations jouees", i, 12000);
	h_true(g_c.accepted > 100, "des mutants sont acceptes et verifies");
	h_true(g_c.accepted < 12000, "des mutants sont refuses");
}

int	main(void)
{
	h_begin("d04 mutations a graine fixe");
	h_run("12000 mutations sans lecture hors tampon", mutations);
	return (h_end());
}
