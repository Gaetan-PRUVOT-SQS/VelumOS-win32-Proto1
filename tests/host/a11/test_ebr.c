#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static uint8_t		g_sec[512];
static t_partlist	g_pl;
static t_ebr		g_e;

static int	step(const t_fkpart *log, const t_fkpart *next)
{
	memset(&g_pl, 0, sizeof(g_pl));
	memset(&g_e, 0, sizeof(g_e));
	g_e.ext_first = 1000;
	g_e.ext_count = 5000;
	g_e.cur = 1000;
	g_e.num = EBR_FIRST_NUM;
	fk_ebr_sector(g_sec, log, next);
	return (ebr_step(g_sec, &g_e, &g_pl));
}

static void	maillon_valide(void)
{
	static const t_fkpart	p[2] = {{63, 1062, 0}, {2000, 2999, 0}};

	h_eq_i64("maillon suivi", step(&p[0], &p[1]), 1);
	h_eq_u64("une logique", g_pl.n, 1);
	h_eq_u64("debut relatif a son EBR", g_pl.p[0].first, 1063);
	h_eq_u64("taille", g_pl.p[0].count, 1000);
	h_eq_u64("numero 5", g_pl.p[0].num, 5);
	h_eq_u64("suivant relatif a l'etendue", g_e.cur, 3000);
	h_eq_u64("numero suivant", g_e.num, 6);
	h_eq_i64("dernier maillon", step(&p[0], NULL), 0);
	h_eq_u64("logique gardee", g_pl.n, 1);
	h_eq_i64("aucune logique, fin de chaine", step(NULL, NULL), 0);
	h_eq_u64("zero logique", g_pl.n, 0);
	h_eq_i64("EBR vide mais lie", step(NULL, &p[1]), 1);
	h_eq_u64("aucune logique ajoutee", g_pl.n, 0);
	h_eq_u64("suivant", g_e.cur, 3000);
}

static void	limites_de_la_logique(void)
{
	static const t_fkpart	p[5] = {{1, 4999, 0}, {1, 5000, 0}, {0, 99, 0},
	{5000, 5000, 0}, {4999, 4999, 0}};

	h_eq_i64("fin exacte de l'etendue", step(&p[0], NULL), 0);
	h_eq_u64("logique gardee", g_pl.n, 1);
	h_eq_i64("deborde d'un secteur", step(&p[1], NULL), E_PROTO);
	h_eq_u64("rien d'ajoute", g_pl.n, 0);
	h_eq_i64("recouvre son EBR", step(&p[2], NULL), E_PROTO);
	h_eq_i64("debut a la fin de l'etendue", step(&p[3], NULL), E_PROTO);
	h_eq_i64("dernier secteur de l'etendue", step(&p[4], NULL), 0);
	h_eq_u64("debut 5999", g_pl.p[0].first, 5999);
	h_eq_u64("un secteur", g_pl.p[0].count, 1);
}

static void	limites_du_lien(void)
{
	static const t_fkpart	p[6] = {{63, 1062, 0}, {0, 99, 0},
	{1062, 1100, 0}, {1063, 1100, 0}, {5000, 5000, 0}, {4999, 4999, 0}};

	h_eq_i64("lien vers soi (boucle)", step(NULL, &p[1]), E_PROTO);
	h_eq_u64("position inchangee", g_e.cur, 1000);
	h_eq_i64("lien dans la logique courante", step(&p[0], &p[2]), E_PROTO);
	h_eq_i64("lien juste apres la logique", step(&p[0], &p[3]), 1);
	h_eq_u64("EBR suivant", g_e.cur, 2063);
	h_eq_i64("lien a la fin de l'etendue", step(NULL, &p[4]), E_PROTO);
	h_eq_i64("lien au dernier secteur", step(NULL, &p[5]), 1);
	h_eq_u64("EBR suivant 5999", g_e.cur, 5999);
	step(NULL, &p[3]);
	g_e.cur = 5999;
	h_eq_i64("retour en arriere (boucle)", ebr_step(g_sec, &g_e, &g_pl),
		E_PROTO);
	h_eq_u64("position gardee", g_e.cur, 5999);
}

int	main(void)
{
	h_begin("a11/ebr");
	h_run("ebr/maillon-valide-et-fin-de-chaine", maillon_valide);
	h_run("ebr/limites-de-la-partition-logique", limites_de_la_logique);
	h_run("ebr/limites-du-lien-et-boucle", limites_du_lien);
	return (h_end());
}
