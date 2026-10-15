#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram		g_r;
static t_partlist	g_pl;
static uint8_t		g_sec[512];
static int			g_reads;

static uint32_t	scan(const t_fkpart *t, int prim_slot, int ext_slot)
{
	fk_ram_init(&g_r, "vdb", 512, 4096);
	fk_mbr_entry(g_r.data, prim_slot, 0x83, &t[0]);
	fk_mbr_entry(g_r.data, ext_slot, 0x05, &t[1]);
	fk_ebr_sector(g_r.data + t[1].first * 512, &t[2], NULL);
	h_eq_i64("table primaire", mbr_parse(g_r.data, 4096, &g_pl), 0);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	g_reads = g_r.reads;
	fk_ram_free(&g_r);
	return (g_pl.n);
}

static void	ebr_dans_une_primaire(void)
{
	static const t_fkpart	t[3] = {{1, 2047, 0}, {1024, 4095, 0},
	{1024, 2047, 0}};
	static const t_fkpart	in[3] = {{3000, 3500, 0}, {1000, 4095, 0},
	{1, 100, 0}};
	static const t_fkpart	out[3] = {{100, 4000, 0}, {1000, 2000, 0},
	{1, 100, 0}};

	h_eq_u64("EBR lu dans vda1 : zero logique", scan(t, 0, 1), 1);
	h_eq_u64("primaire gardee", g_pl.p[0].num, 1);
	h_eq_u64("debut de la primaire", g_pl.p[0].first, 1);
	h_eq_i64("aucun EBR lu", g_reads, 0);
	h_eq_u64("etendue en entree 1, primaire en entree 4", scan(t, 3, 0), 1);
	h_eq_u64("primaire numero 4", g_pl.p[0].num, 4);
	h_eq_i64("aucun EBR lu (ordre inverse)", g_reads, 0);
	h_eq_u64("primaire dans l'etendue", scan(in, 0, 1), 1);
	h_eq_i64("aucun EBR lu (primaire dedans)", g_reads, 0);
	h_eq_u64("etendue dans une primaire", scan(out, 0, 1), 1);
	h_eq_i64("aucun EBR lu (etendue dedans)", g_reads, 0);
}

static void	limites_du_chevauchement(void)
{
	static const t_fkpart	a[3] = {{100, 1000, 0}, {1000, 4095, 0},
	{1, 100, 0}};
	static const t_fkpart	b[3] = {{2000, 3000, 0}, {1000, 2000, 0},
	{1, 100, 0}};
	static const t_fkpart	c[3] = {{100, 999, 0}, {1000, 2000, 0},
	{1, 100, 0}};
	static const t_fkpart	d[3] = {{2000, 3000, 0}, {1000, 1999, 0},
	{1, 100, 0}};

	h_eq_u64("un secteur commun au debut de l'etendue", scan(a, 0, 1), 1);
	h_eq_i64("aucun EBR lu (debut)", g_reads, 0);
	h_eq_u64("un secteur commun a la fin de l'etendue", scan(b, 0, 1), 1);
	h_eq_i64("aucun EBR lu (fin)", g_reads, 0);
	h_eq_u64("primaire adjacente avant : suivie", scan(c, 0, 1), 2);
	h_eq_u64("logique exposee", g_pl.p[1].first, 1001);
	h_eq_u64("primaire adjacente apres : suivie", scan(d, 3, 0), 2);
	h_eq_i64("un EBR lu", g_reads, 1);
}

int	main(void)
{
	h_begin("a11/ebr_primaires");
	h_run("ebr/etendue-qui-chevauche-une-primaire", ebr_dans_une_primaire);
	h_run("ebr/chevauchement-d-un-secteur-et-adjacence",
		limites_du_chevauchement);
	return (h_end());
}
