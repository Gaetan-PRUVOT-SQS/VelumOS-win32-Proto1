#include <stdio.h>
#include <stdlib.h>
#include "display_int.h"
#include "fuzz_table.h"
#include "velum/err.h"
#include "velum/uptr.h"

static uint64_t	env_u64(const char *name, uint64_t fallback)
{
	const char	*v;
	char		*end;
	uint64_t	n;

	v = getenv(name);
	if (!v || !*v)
		return (fallback);
	n = strtoull(v, &end, 0);
	if (*end)
		return (fallback);
	return (n);
}

static void	print_run(const t_fzrun *r)
{
	uint32_t	i;

	printf("a13/sys_fuzz_table : %llu appels, %llu reussis, %llu refuses, "
		"%llu E_NOSYS, %llu incidents\n",
		(unsigned long long)r->calls, (unsigned long long)r->ok,
		(unsigned long long)r->refused, (unsigned long long)r->nosys,
		(unsigned long long)r->incidents);
	i = 0;
	while (i < r->registered_count)
	{
		printf("a13/sys_fuzz_table : numero %#x appele %llu fois\n",
			r->registered[i],
			(unsigned long long)r->by_num[r->registered[i]]);
		i++;
	}
}

static void	run_fresh(t_fzrun *r, uint64_t seed, uint64_t calls)
{
	static uint8_t	buf[FZ_BUF];

	fx_display(1024, 768);
	fz_open(r, seed, buf);
	fz_run(r, calls, 0);
}

static void	reproducible(void)
{
	t_fzrun	a;
	t_fzrun	b;
	t_fzrun	c;

	run_fresh(&a, 4242, 5000);
	run_fresh(&b, 4242, 5000);
	run_fresh(&c, 4243, 5000);
	h_eq_u64("meme graine, meme nombre de reussites", a.ok, b.ok);
	h_eq_u64("meme graine, meme nombre de refus", a.refused, b.refused);
	h_eq_u64("meme graine, meme nombre de E_NOSYS", a.nosys, b.nosys);
	h_true(memcmp(a.by_num, b.by_num, sizeof(a.by_num)) == 0,
		"meme graine, memes numeros appeles");
	h_true(memcmp(a.by_num, c.by_num, sizeof(a.by_num)) != 0,
		"graine voisine, suite d'appels differente");
}

static void	seeds_spread(void)
{
	static const uint64_t	seeds[] = {1, 2, 3, 0xffffffffull, 20261015ull,
		0x9e3779b97f4a7c15ull};
	t_fzrun					r;
	size_t					i;

	i = 0;
	while (i < sizeof(seeds) / sizeof(seeds[0]))
	{
		run_fresh(&r, seeds[i], 8000);
		h_eq_u64("aucun incident pour la graine", r.incidents, 0);
		i++;
	}
}

static void	oracle(void)
{
	static uint8_t	buf[FZ_BUF];
	t_fzrun			r;
	t_fzcall		c;

	fx_display(1024, 768);
	fz_open(&r, 7, buf);
	memset(&c, 0, sizeof(c));
	c.num = SYS_DISPLAY_INFO;
	g_fp.locks = 0;
	g_fp.unlocks = 0;
	h_true(fz_verdict(&r, &c, 0) == NULL, "retour 0 accepte");
	h_true(fz_verdict(&r, &c, E_FAULT) == NULL, "E_FAULT accepte");
	h_true(fz_verdict(&r, &c, UPTR_E2BIG) == NULL
		|| fz_documented(UPTR_E2BIG), "E2BIG fait partie des codes valides");
	h_true(fz_verdict(&r, &c, -3) != NULL, "code -3 refuse");
	h_true(fz_verdict(&r, &c, E_NOSYS) != NULL,
		"E_NOSYS refuse sur un appel enregistre");
	c.num = 0x7f;
	h_true(fz_verdict(&r, &c, E_FAULT) != NULL,
		"numero absent sans E_NOSYS refuse");
	h_true(fz_verdict(&r, &c, E_NOSYS) == NULL, "numero absent avec E_NOSYS");
	c.num = SYS_DISPLAY_MAP;
	h_true(fz_verdict(&r, &c, 0x1000001) != NULL,
		"adresse de mappage non alignee refusee");
	c.num = SYS_DISPLAY_INFO;
	g_fp.locks = 1;
	h_true(fz_verdict(&r, &c, 0) != NULL, "verrou non rendu detecte");
	g_fp.locks = 0;
	buf[FZ_WRITABLE] = 0;
	h_true(fz_verdict(&r, &c, 0) != NULL, "ecriture interdite detectee");
	buf[FZ_WRITABLE] = 0xaa;
}

static void	golden_table(void)
{
	static uint8_t	buf[FZ_BUF];
	t_fzrun			r;
	t_sysfn			saved;
	t_sysfn			kcon;

	fx_display(1024, 768);
	fz_open(&r, 11, buf);
	h_true(r.gactive > 40, "liste figee lue avec plus de 40 appels actifs");
	h_true(fz_check_table(&r) == NULL,
		"table enregistree conforme a la liste figee");
	h_eq_u64("SYS_DISPLAY_INFO actif et du lot", r.gstate[SYS_DISPLAY_INFO],
		FZ_ACTIVE);
	h_eq_u64("0x65 declare en trou", r.gstate[0x65], FZ_HOLE);
	h_eq_u64("0x0b declare en trou", r.gstate[0x0b], FZ_HOLE);
	h_eq_u64("SYS_EXIT actif hors du lot", r.gstate[SYS_EXIT], FZ_ACTIVE);
	h_eq_u64("SYS_EXIT n'appartient pas au lot", r.gowned[SYS_EXIT], 0);
	saved = g_fp.sys[SYS_DISPLAY_INFO];
	g_fp.sys[0x65] = saved;
	h_true(fz_check_table(&r) != NULL, "appel enregistre dans un trou detecte");
	g_fp.sys[0x65] = NULL;
	g_fp.sys[SYS_EXIT] = saved;
	h_true(fz_check_table(&r) != NULL,
		"appel enregistre hors du lot detecte");
	g_fp.sys[SYS_EXIT] = NULL;
	kcon = g_fp.sys[SYS_KCON];
	g_fp.sys[SYS_KCON] = NULL;
	h_true(fz_check_table(&r) != NULL, "appel du lot non enregistre detecte");
	g_fp.sys[SYS_KCON] = kcon;
	h_true(fz_check_table(&r) == NULL, "table restauree conforme");
}

static void	contract_oracle(void)
{
	static uint8_t	buf[FZ_BUF];
	t_fzrun			r;
	t_fzcall		c;

	fx_display(1024, 768);
	fz_open(&r, 3, buf);
	memset(&c, 0, sizeof(c));
	c.pid = 10;
	c.flags = PF_DISPLAY;
	g_fp.locks = 0;
	g_fp.unlocks = 0;
	c.num = SYS_DISPLAY_INFO;
	h_true(fz_verdict(&r, &c, E_PERM) != NULL, "INFO ne rend jamais E_PERM");
	c.num = SYS_DISPLAY_SET_MODE;
	c.flags = 0;
	h_true(fz_verdict(&r, &c, E_PERM) == NULL, "SET_MODE sans droit : E_PERM");
	h_true(fz_verdict(&r, &c, 0) != NULL, "SET_MODE sans droit : succes refuse");
	c.flags = PF_DISPLAY;
	h_true(fz_verdict(&r, &c, E_PERM) != NULL,
		"SET_MODE avec droit : E_PERM refuse");
	c.a[1] = 0x100000000ull;
	h_true(fz_verdict(&r, &c, E_INVAL) == NULL,
		"SET_MODE argument 64 bits : E_INVAL");
	h_true(fz_verdict(&r, &c, E_NODEV) != NULL,
		"SET_MODE argument 64 bits : autre code refuse");
	c.num = SYS_KCON;
	c.a[0] = 2;
	h_true(fz_verdict(&r, &c, E_INVAL) == NULL, "KCON 2 : E_INVAL");
	h_true(fz_verdict(&r, &c, 0) != NULL, "KCON 2 : succes refuse");
	c.no_proc = true;
	h_true(fz_verdict(&r, &c, E_PERM) == NULL, "KCON sans processus : E_PERM");
	c.no_proc = false;
	c.num = SYS_DISPLAY_MAP;
	c.flags = 0;
	h_true(fz_verdict(&r, &c, E_FAULT) != NULL, "MAP sans droit : E_FAULT refuse");
	c.num = SYS_DISPLAY_MODES;
	c.flags = PF_DISPLAY;
	c.a[1] = 0x100000000ull;
	h_true(fz_verdict(&r, &c, E_INVAL) == NULL, "MODES 64 bits : E_INVAL");
	c.a[1] = 8;
	h_true(fz_verdict(&r, &c, 9) != NULL, "MODES plus de 8 modes refuse");
	h_true(fz_verdict(&r, &c, 8) == NULL, "MODES 8 modes accepte");
}

static void	sweep_table(void)
{
	static uint8_t	buf[FZ_BUF];
	t_fzrun			r;
	uint32_t		n;
	uint32_t		missed;

	fx_display(1024, 768);
	fz_open(&r, 99, buf);
	h_true(fz_sweep(&r), "balayage de toute la table sans incident");
	h_eq_u64("aucun incident au balayage", r.incidents, 0);
	missed = 0;
	n = 0;
	while (n < FZ_SLOTS)
		missed += r.by_num[n++] == 0;
	h_eq_u64("les 256 numeros sont tous appeles", missed, 0);
	h_true(r.outside >= FZ_OUTSIDE_COUNT * 100,
		"numeros hors table appeles avec arguments pathologiques");
	h_true(r.nosys > r.ok, "la plupart des numeros du lot repondent E_NOSYS");
	h_true(r.ok > 0, "le balayage fait reussir des appels du lot");
}

static void	fuzz_table(void)
{
	static uint8_t	buf[FZ_BUF];
	t_fzrun			r;
	uint64_t		seed;
	uint64_t		calls;
	uint64_t		seconds;
	uint32_t		i;

	seed = env_u64("VELUM_FUZZ_SEED", 20261015);
	calls = env_u64("VELUM_FUZZ_CALLS", 60000);
	seconds = env_u64("VELUM_FUZZ_SECONDS", 0);
	printf("a13/sys_fuzz_table : graine %llu appels %llu secondes %llu\n",
		(unsigned long long)seed, (unsigned long long)calls,
		(unsigned long long)seconds);
	h_true(fx_display(1024, 768), "affichage pret avant le fuzz");
	fz_open(&r, seed, buf);
	h_true(r.registered_count >= 5, "la table contient les appels d'affichage");
	i = 0;
	while (i < r.registered_count)
		h_true(r.registered[i++] < SYS_MAX, "numero inscrit sous SYS_MAX");
	h_true(fz_run(&r, calls, seconds), "aucun incident sur toute la table");
	print_run(&r);
	h_eq_u64("aucun incident", r.incidents, 0);
	h_true(r.nosys > 0, "les numeros absents repondent E_NOSYS");
	h_true(r.ok > 0, "au moins un appel reussit");
}

int	main(void)
{
	h_begin("a13/sys_fuzz_table");
	h_run("oracle du fuzz de table", oracle);
	h_run("liste figee et table enregistree", golden_table);
	h_run("contrat par appel", contract_oracle);
	h_run("balayage de tous les numeros", sweep_table);
	h_run("graine reproductible", reproducible);
	h_run("plusieurs graines", seeds_spread);
	h_run("fuzz de toute la table des appels", fuzz_table);
	return (h_end());
}
