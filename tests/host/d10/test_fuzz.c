#include <stdlib.h>
#include "fake.h"

static t_fuzz	g_z;

static uint32_t	rnd(void)
{
	g_z.seed ^= g_z.seed << 13;
	g_z.seed ^= g_z.seed >> 17;
	g_z.seed ^= g_z.seed << 5;
	return (g_z.seed);
}

static void	check_text(const uint8_t *src, size_t len)
{
	uint8_t	*copy;
	t_span	s;
	t_text	out;
	int		n;

	copy = malloc(len);
	if (len)
		memcpy(copy, src, len);
	s.p = copy;
	s.len = len;
	n = pm_registry_parse(s, g_z.e, 8);
	free(copy);
	h_true(n >= 0 && n <= 8, "analyse bornee");
	out.p = g_z.text;
	out.cap = sizeof(g_z.text);
	s.len = (size_t)pm_registry_format(g_z.e, (uint32_t)n, out);
	s.p = (const uint8_t *)g_z.text;
	h_true(pm_registry_parse(s, g_z.e, 8) == n, "accepte donc reecrit et relu");
}

static void	t_mutations(void)
{
	uint8_t	work[4096];
	size_t	len;
	int		i;
	int		k;

	len = (size_t)fake_base((char *)g_z.base, sizeof(g_z.base));
	g_z.seed = 0x0d10c0de;
	i = 0;
	while (i < 10000)
	{
		memcpy(work, g_z.base, len);
		k = 1 + (int)(rnd() % 4);
		while (k > 0)
		{
			work[rnd() % len] = (uint8_t)rnd();
			k--;
		}
		check_text(work, len - (rnd() % 8 == 0) * (rnd() % len));
		i++;
	}
}

static void	t_truncation(void)
{
	size_t	len;
	size_t	cut;

	len = (size_t)fake_base((char *)g_z.base, sizeof(g_z.base));
	cut = 0;
	while (cut <= len)
	{
		check_text(g_z.base, cut);
		cut++;
	}
}

int	main(void)
{
	h_begin("d10 registre hostile");
	h_run("registre_10000_mutations_graine_fixe", t_mutations);
	h_run("registre_troncature_a_chaque_octet", t_truncation);
	return (h_end());
}
