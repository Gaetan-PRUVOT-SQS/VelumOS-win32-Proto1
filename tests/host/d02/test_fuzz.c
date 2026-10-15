#include "harness.h"
#include "fx.h"

static void	trunc_one(const char *name, int kind)
{
	t_span	full;
	t_span	cut;
	size_t	n;
	int		bad;

	full = fx_load(name);
	h_true(full.len > 0, name);
	bad = 0;
	n = 0;
	while (n < full.len)
	{
		cut = fx_dup(full, n);
		if (fx_walk(kind, cut) >= 0)
			bad++;
		fx_free(cut);
		n++;
	}
	h_eq_i64("prefixes acceptes", bad, 0);
	h_eq_i64("entier accepte", fx_walk(kind, full), 0);
	fx_free(full);
}

static void	troncatures(void)
{
	trunc_one("pool_u8.bin", FX_POOL);
	trunc_one("pool_u16.bin", FX_POOL);
	trunc_one("manifest.axml", FX_AXML);
	trunc_one("manifest_u8.axml", FX_AXML);
	trunc_one("resources.arsc", FX_ARSC);
}

static void	mutate_one(const char *name, int kind, uint32_t seed)
{
	t_span		full;
	t_span		m;
	uint32_t	i;
	uint32_t	k;
	int			stuck;

	full = fx_load(name);
	stuck = 0;
	i = 0;
	while (i < 4000 && full.len)
	{
		m = fx_dup(full, full.len);
		k = 1 + fx_rnd(&seed) % 3;
		while (k--)
			fx_poke(m, fx_rnd(&seed) % m.len, (uint8_t)fx_rnd(&seed));
		stuck += fx_walk(kind, m) > 0;
		fx_free(m);
		i++;
	}
	h_eq_u64(name, i, 4000);
	h_eq_i64("parcours sans fin", stuck, 0);
	fx_free(full);
}

static void	mutations(void)
{
	mutate_one("pool_u8.bin", FX_POOL, 0x9e3779b9);
	mutate_one("pool_u16.bin", FX_POOL, 0x12345678);
	mutate_one("manifest.axml", FX_AXML, 0xdeadbeef);
	mutate_one("manifest_u8.axml", FX_AXML, 0x0badcafe);
	mutate_one("resources.arsc", FX_ARSC, 0x00c0ffee);
}

int	main(void)
{
	h_begin("d02 mutations");
	h_run("troncature_chaque_octet", troncatures);
	h_run("mutations_graine_fixe", mutations);
	return (h_end());
}
