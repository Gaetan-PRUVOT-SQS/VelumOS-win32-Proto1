#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static uint8_t		g_sec[512];
static t_partlist	g_pl;
static t_ebr		g_e;

static int	step_at(uint64_t ext_first, uint64_t ext_count)
{
	memset(&g_pl, 0, sizeof(g_pl));
	memset(&g_e, 0, sizeof(g_e));
	g_e.ext_first = ext_first;
	g_e.ext_count = ext_count;
	g_e.cur = ext_first;
	g_e.num = EBR_FIRST_NUM;
	return (ebr_step(g_sec, &g_e, &g_pl));
}

static void	signature_et_types(void)
{
	static const t_fkpart	p[2] = {{63, 1062, 0}, {2000, 2999, 0}};

	fk_ebr_sector(g_sec, &p[0], &p[1]);
	g_sec[511] = 0;
	h_eq_i64("signature fausse (0xAA)", step_at(1000, 5000), E_PROTO);
	h_eq_u64("rien d'ajoute", g_pl.n, 0);
	g_sec[511] = 0xaa;
	g_sec[510] = 0;
	h_eq_i64("signature fausse (0x55)", step_at(1000, 5000), E_PROTO);
	g_sec[510] = 0x55;
	g_sec[446 + 4] = 0x0f;
	h_eq_i64("etendue en premiere entree", step_at(1000, 5000), E_PROTO);
	g_sec[446 + 4] = 0x83;
	g_sec[462 + 4] = 0x85;
	h_eq_i64("lien de type 0x85", step_at(1000, 5000), 1);
	g_sec[462 + 4] = 0x83;
	h_eq_i64("seconde entree non etendue, fin", step_at(1000, 5000), 0);
	h_eq_u64("logique gardee", g_pl.n, 1);
}

static void	liste_et_chevauchement(void)
{
	static const t_fkpart	p[1] = {{63, 1062, 0}};
	static const t_part		prim = {1500, 100, 1};
	uint32_t				i;

	fk_ebr_sector(g_sec, &p[0], NULL);
	step_at(1000, 5000);
	g_pl.n = 1;
	g_pl.p[0] = prim;
	h_eq_i64("chevauche une primaire", ebr_step(g_sec, &g_e, &g_pl), E_PROTO);
	h_eq_u64("liste inchangee", g_pl.n, 1);
	i = 0;
	while (i < BLK_PARTS_MAX)
	{
		g_pl.p[i].first = 10000 + 10 * i;
		g_pl.p[i].count = 5;
		g_pl.p[i].num = i + 1;
		i++;
	}
	g_pl.n = BLK_PARTS_MAX;
	h_eq_i64("liste pleine", ebr_step(g_sec, &g_e, &g_pl), E_NOTSUP);
	h_eq_u64("pas de depassement", g_pl.n, BLK_PARTS_MAX);
}

static void	additions_sans_debordement(void)
{
	static const t_fkpart	p[4] = {{0x200, 0x11ff, 0},
	{0x0ffffffffull, 0x0ffffffffull, 0},
	{0x0fffffffeull, 0x1fffffffcull, 0},
	{0x0fffffffeull, 0x0fffffffeull, 0}};

	fk_ebr_sector(g_sec, &p[0], NULL);
	h_eq_i64("au-dela de 2^32", step_at(0x0ffffff00ull, 0x0ffffffffull), 0);
	h_eq_u64("somme sur 64 bits", g_pl.p[0].first, 0x100000100ull);
	fk_ebr_sector(g_sec, &p[1], NULL);
	h_eq_i64("decalage maximal", step_at(0x0ffffff00ull, 0x0ffffffffull),
		E_PROTO);
	fk_ebr_sector(g_sec, &p[2], NULL);
	h_eq_i64("taille maximale", step_at(0x0ffffff00ull, 0x0ffffffffull),
		E_PROTO);
	fk_ebr_sector(g_sec, &p[3], NULL);
	h_eq_i64("dernier secteur", step_at(0x0ffffff00ull, 0x0ffffffffull), 0);
	h_eq_u64("debut exact", g_pl.p[0].first, 0x1fffffefeull);
	fk_ebr_sector(g_sec, NULL, &p[1]);
	h_eq_i64("lien maximal", step_at(0x0ffffff00ull, 0x0ffffffffull),
		E_PROTO);
}

int	main(void)
{
	h_begin("a11/ebr_hostile");
	h_run("ebr/signature-et-types", signature_et_types);
	h_run("ebr/liste-pleine-et-chevauchement", liste_et_chevauchement);
	h_run("ebr/additions-de-lba-sans-debordement",
		additions_sans_debordement);
	return (h_end());
}
