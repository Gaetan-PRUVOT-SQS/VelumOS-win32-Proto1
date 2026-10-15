#include <stdlib.h>
#include "harness.h"
#include "../../../kernel/stackguard_seed.h"
#include "../../../arch/x86_64/boot_guard.h"

#define MODEL_RDRAND_ESSAIS 10
#define MODEL_RDSEED_ESSAIS 64
#define MODEL_REPLI 0x595e9fbd94fda700ULL

typedef struct s_script
{
	unsigned int	rdrand_fail_first;
	unsigned int	rdseed_fail_first;
	uint64_t		rdrand_value;
	uint64_t		rdseed_value;
	uint64_t		tsc_a;
	uint64_t		tsc_b;
	unsigned int	rdrand_calls;
	unsigned int	rdseed_calls;
	unsigned int	tsc_calls;
}	t_script;

typedef struct s_modele
{
	uint64_t	valeur;
	t_guardsrc	src;
	unsigned int	rdrand_calls;
	unsigned int	rdseed_calls;
}	t_modele;

static t_script	g_s;

static bool	fake_rdrand(uint64_t *out)
{
	g_s.rdrand_calls++;
	*out = g_s.rdrand_value;
	return (g_s.rdrand_calls > g_s.rdrand_fail_first);
}

static bool	fake_rdseed(uint64_t *out)
{
	g_s.rdseed_calls++;
	*out = g_s.rdseed_value;
	return (g_s.rdseed_calls > g_s.rdseed_fail_first);
}

static uint64_t	fake_tsc(void)
{
	g_s.tsc_calls++;
	if (g_s.tsc_calls == 1)
		return (g_s.tsc_a);
	return (g_s.tsc_b);
}

static t_guardhw	hw_make(bool rdrand, bool rdseed)
{
	t_guardhw	hw;

	hw.has_rdrand = rdrand;
	hw.has_rdseed = rdseed;
	hw.rdrand = fake_rdrand;
	hw.rdseed = fake_rdseed;
	hw.tsc = fake_tsc;
	return (hw);
}

static void	scenario(unsigned int rdrand_fail, unsigned int rdseed_fail)
{
	g_s = (t_script){0};
	g_s.rdrand_fail_first = rdrand_fail;
	g_s.rdseed_fail_first = rdseed_fail;
	g_s.rdrand_value = 0x1122334455667788ULL;
	g_s.rdseed_value = 0x8877665544332211ULL;
	g_s.tsc_a = 0x0000000100000002ULL;
	g_s.tsc_b = 0x0000000300000004ULL;
}

static uint64_t	ref_splitmix(uint64_t z)
{
	static const struct
	{
		unsigned int	decalage;
		uint64_t		facteur;
	}	etapes[2] = {{30, 0xbf58476d1ce4e5b9ULL}, {27, 0x94d049bb133111ebULL}};
	unsigned int	i;

	for (i = 0; i < 2; i++)
	{
		z ^= z >> etapes[i].decalage;
		z *= etapes[i].facteur;
	}
	return (z ^ (z >> 31));
}

static bool	ref_tirage_ok(uint64_t v)
{
	return (v != 0 && v != 0xffffffffffffffffULL);
}

static unsigned int	ref_appels(unsigned int echecs_avant, unsigned int essais,
	uint64_t valeur, bool *obtenu)
{
	*obtenu = false;
	if (!ref_tirage_ok(valeur) || echecs_avant >= essais)
		return (essais);
	*obtenu = true;
	return (echecs_avant + 1);
}

static t_modele	ref_modele(bool rdrand, bool rdseed)
{
	t_modele	m;
	bool		a_seed;
	bool		a_rand;
	uint64_t	dur;
	uint64_t	z;

	m = (t_modele){0};
	a_seed = false;
	a_rand = false;
	if (rdseed)
		m.rdseed_calls = ref_appels(g_s.rdseed_fail_first,
				MODEL_RDSEED_ESSAIS, g_s.rdseed_value, &a_seed);
	if (rdrand)
		m.rdrand_calls = ref_appels(g_s.rdrand_fail_first,
				MODEL_RDRAND_ESSAIS, g_s.rdrand_value, &a_rand);
	dur = 0;
	m.src = GUARD_SRC_TSC;
	if (a_seed && a_rand && g_s.rdseed_value != g_s.rdrand_value)
	{
		m.src = GUARD_SRC_BOTH;
		dur = g_s.rdseed_value ^ g_s.rdrand_value;
	}
	else if (a_seed)
	{
		m.src = GUARD_SRC_RDSEED;
		dur = g_s.rdseed_value;
	}
	else if (a_rand)
	{
		m.src = GUARD_SRC_RDRAND;
		dur = g_s.rdrand_value;
	}
	z = dur ^ g_s.tsc_a ^ ((g_s.tsc_b << 32) | (g_s.tsc_b >> 32));
	m.valeur = ref_splitmix(z) & 0xffffffffffffff00ULL;
	if (m.valeur == 0)
		m.valeur = MODEL_REPLI;
	return (m);
}

static void	contrat_des_constantes(void)
{
	h_eq_u64("essais rdrand : dix (SDM vol. 1, 7.3.17.1)", GUARD_RDRAND_TRIES,
		MODEL_RDRAND_ESSAIS);
	h_eq_u64("essais rdseed bornés", GUARD_RDSEED_TRIES, MODEL_RDSEED_ESSAIS);
	h_eq_u64("valeur de repli", GUARD_FALLBACK, MODEL_REPLI);
}

static void	un_cas(bool rdrand, bool rdseed, const char *nom)
{
	t_guardhw	hw;
	t_guardsrc	src;
	t_modele	m;
	uint64_t	g;

	m = ref_modele(rdrand, rdseed);
	hw = hw_make(rdrand, rdseed);
	g = guard_seed(&hw, &src);
	h_eq_u64(nom, g, m.valeur);
	h_true(src == m.src, nom);
	h_eq_u64(nom, g_s.rdrand_calls, m.rdrand_calls);
	h_eq_u64(nom, g_s.rdseed_calls, m.rdseed_calls);
	h_eq_u64(nom, g_s.tsc_calls, 2);
	h_eq_u64(nom, g & 0xff, 0);
	h_true(g != 0, nom);
}

static void	table_de_decision(void)
{
	static const unsigned int	echecs_rand[] = {0, 1, 9, 10, 11};
	static const unsigned int	echecs_seed[] = {0, 1, 63, 64, 65};
	static const uint64_t		valeurs[] = {0x1122334455667788ULL, 0,
		0xffffffffffffffffULL, 0x00000000ffffffffULL};
	unsigned int	a;
	unsigned int	b;
	unsigned int	c;
	unsigned int	d;
	unsigned int	cpuid;

	for (cpuid = 0; cpuid < 4; cpuid++)
		for (a = 0; a < 5; a++)
			for (b = 0; b < 5; b++)
				for (c = 0; c < 4; c++)
					for (d = 0; d < 4; d++)
					{
						scenario(echecs_rand[a], echecs_seed[b]);
						g_s.rdrand_value = valeurs[c];
						g_s.rdseed_value = valeurs[d] ^ (d == 0 ? 0x1ULL : 0);
						un_cas(cpuid & 1, cpuid & 2, "table rdrand x rdseed");
					}
}

static void	cf_nul_valeur_ignoree(void)
{
	t_guardhw	hw;
	t_guardsrc	src;
	uint64_t	g;
	uint64_t	seul_tsc;

	scenario(MODEL_RDRAND_ESSAIS, MODEL_RDSEED_ESSAIS);
	hw = hw_make(true, true);
	seul_tsc = guard_seed(&hw, &src);
	h_true(src == GUARD_SRC_TSC, "tout échoue : tsc seul");
	scenario(MODEL_RDRAND_ESSAIS, MODEL_RDSEED_ESSAIS);
	g_s.rdrand_value = 0xdeadbeefcafef00dULL;
	g_s.rdseed_value = 0x0123456789abcdefULL;
	hw = hw_make(true, true);
	g = guard_seed(&hw, &src);
	h_eq_u64("valeurs lues avec CF=0 jamais utilisées", g, seul_tsc);
	h_eq_u64("rdseed borné", g_s.rdseed_calls, MODEL_RDSEED_ESSAIS);
	h_eq_u64("rdrand borné", g_s.rdrand_calls, MODEL_RDRAND_ESSAIS);
}

static void	sources_identiques(void)
{
	t_guardhw	hw;
	t_guardsrc	src;

	scenario(0, 0);
	g_s.rdseed_value = g_s.rdrand_value;
	hw = hw_make(true, true);
	guard_seed(&hw, &src);
	h_true(src == GUARD_SRC_RDSEED, "deux sources égales : pas de xor nul");
	scenario(0, 0);
	hw = hw_make(true, true);
	guard_seed(&hw, &src);
	h_true(src == GUARD_SRC_BOTH, "deux sources distinctes mélangées");
}

static void	forme_et_repli(void)
{
	t_guardhw	hw;
	t_guardsrc	src;

	h_eq_u64("octet nul forcé", guard_shape(0x1234567890abcdefULL),
		0x1234567890abcd00ULL);
	h_eq_u64("zéro après masque : repli", guard_shape(0xffULL), MODEL_REPLI);
	h_eq_u64("zéro : repli", guard_shape(0), MODEL_REPLI);
	scenario(MODEL_RDRAND_ESSAIS, MODEL_RDSEED_ESSAIS);
	g_s.tsc_a = 0;
	g_s.tsc_b = 0;
	hw = hw_make(true, true);
	h_eq_u64("tsc nul sans matériel : jamais zéro",
		guard_seed(&hw, &src) != 0, 1);
}

static void	diffusion_du_tsc(void)
{
	t_guardhw	hw;
	t_guardsrc	src;
	uint64_t	base;
	unsigned int	i;
	unsigned int	differents;

	scenario(MODEL_RDRAND_ESSAIS, MODEL_RDSEED_ESSAIS);
	hw = hw_make(false, false);
	base = guard_seed(&hw, &src);
	differents = 0;
	for (i = 0; i < 64; i++)
	{
		scenario(MODEL_RDRAND_ESSAIS, MODEL_RDSEED_ESSAIS);
		g_s.tsc_a ^= 1ULL << i;
		hw = hw_make(false, false);
		if (guard_seed(&hw, &src) != base)
			differents++;
	}
	h_true(differents >= 60, "un bit du tsc change le canari");
}

static uint64_t	*g_pages[4];

static uint64_t	*page_alloc(void)
{
	return (aligned_alloc(4096, 4096));
}

static void	garde_de_pile(void)
{
	uint64_t	*pml4;
	uint64_t	va;
	uint64_t	*pte;
	int			i;

	for (i = 0; i < 4; i++)
	{
		g_pages[i] = page_alloc();
		for (int j = 0; j < 512; j++)
			g_pages[i][j] = 0;
	}
	va = 0xffffffff80080000ULL;
	pml4 = g_pages[0];
	pml4[(va >> 39) & 511] = (uint64_t)g_pages[1] | 1;
	g_pages[1][(va >> 30) & 511] = (uint64_t)g_pages[2] | 1;
	g_pages[2][(va >> 21) & 511] = (uint64_t)g_pages[3] | 1;
	g_pages[3][(va >> 12) & 511] = 0x200000 | 3;
	g_pages[3][((va >> 12) & 511) + 1] = 0x201000 | 3;
	h_eq_i64("garde retirée", bg_unmap(0, (uint64_t)pml4, va), 0);
	pte = &g_pages[3][(va >> 12) & 511];
	h_true(!(*pte & 1) && (*pte & BG_PTE_ADDR) == 0x200000,
		"présent à zéro, adresse conservée");
	h_true(g_pages[3][((va >> 12) & 511) + 1] & 1, "page voisine intacte");
	h_eq_i64("déjà retirée", bg_unmap(0, (uint64_t)pml4, va), E_NOTSUP);
	h_eq_i64("adresse non alignée", bg_unmap(0, (uint64_t)pml4, va + 8),
		E_INVAL);
	g_pages[2][(va >> 21) & 511] |= BG_PTE_HUGE;
	h_eq_i64("page large refusée", bg_unmap(0, (uint64_t)pml4, va),
		E_NOTSUP);
	for (i = 0; i < 4; i++)
		free(g_pages[i]);
}

int	main(void)
{
	h_begin("a01/stackguard");
	h_run("contrat des constantes", contrat_des_constantes);
	h_run("table de décision sources x échecs x valeurs", table_de_decision);
	h_run("CF nul : valeur ignorée", cf_nul_valeur_ignoree);
	h_run("sources identiques", sources_identiques);
	h_run("forme et repli", forme_et_repli);
	h_run("diffusion du tsc", diffusion_du_tsc);
	h_run("garde de pile retirée de la table", garde_de_pile);
	return (h_end());
}
