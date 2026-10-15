#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram		g_r;
static t_partlist	g_pl;
static uint8_t		g_sec[512];

static void	disk(const char *name, uint32_t logicals, uint64_t ext_first)
{
	t_fkpart	ext;

	ext.first = ext_first;
	ext.last = ext_first + 3999;
	fk_ram_init(&g_r, name, 512, 20000);
	fk_mbr_entry(g_r.data, 1, 0x0f, &ext);
	if (ext.last < 20000)
		fk_ebr_chain(&g_r, ext_first, logicals, 100);
	h_eq_i64("table primaire", mbr_parse(g_r.data, 20000, &g_pl), 0);
}

static void	nombre_de_logiques(void)
{
	disk("vdb", 0, 1000);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("zero logique", g_pl.n, 0);
	fk_ram_free(&g_r);
	disk("vdb", 1, 1000);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("une logique", g_pl.n, 1);
	h_eq_u64("numero 5", g_pl.p[0].num, 5);
	h_eq_u64("debut", g_pl.p[0].first, 1001);
	h_eq_u64("taille", g_pl.p[0].count, 99);
	fk_ram_free(&g_r);
	disk("vdb", EBR_LINKS_MAX, 1000);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("maximum", g_pl.n, EBR_LINKS_MAX);
	h_eq_u64("dernier numero", g_pl.p[EBR_LINKS_MAX - 1].num, 36);
	h_eq_u64("dernier debut", g_pl.p[EBR_LINKS_MAX - 1].first, 4101);
	fk_ram_free(&g_r);
	disk("vdb", EBR_LINKS_MAX + 1, 1000);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("maximum + 1 : chaine coupee", g_pl.n, EBR_LINKS_MAX);
	h_eq_i64("pas de lecture au-dela", g_r.reads, EBR_LINKS_MAX);
	fk_ram_free(&g_r);
}

static void	chaine_interrompue(void)
{
	static const t_fkpart	p[2] = {{1, 99, 0}, {0, 99, 0}};

	disk("vdb", 5, 1000);
	g_r.fail_at = 1200;
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("erreur de lecture : deux gardees", g_pl.n, 2);
	h_eq_i64("trois lectures", g_r.reads, 3);
	fk_ram_free(&g_r);
	disk("vdb", 5, 1000);
	g_r.data[1200 * 512 + 511] = 0;
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("signature fausse : deux gardees", g_pl.n, 2);
	fk_ram_free(&g_r);
	disk("vdb", 5, 1000);
	fk_ebr_sector(g_r.data + 1200 * 512, &p[0], &p[1]);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("boucle : trois gardees", g_pl.n, 3);
	h_eq_i64("boucle : trois lectures, pas plus", g_r.reads, 3);
	fk_ram_free(&g_r);
	disk("vdb", 2, 16001);
	ebr_scan(&g_r.dev, g_sec, &g_pl);
	h_eq_u64("etendue hors disque : non suivie", g_pl.n, 0);
	h_eq_i64("aucune lecture hors disque", g_r.reads, 0);
	fk_ram_free(&g_r);
}

static void	bout_en_bout(void)
{
	static const t_fkpart	prim = {63, 999, 0};

	disk("vdc", 2, 1000);
	fk_mbr_entry(g_r.data, 0, 0x83, &prim);
	h_eq_i64("enregistrement", blk_register(&g_r.dev), 0);
	h_eq_i64("une primaire et deux logiques", blk_scan_partitions(&g_r.dev),
		3);
	h_true(blk_find("vdc1") && blk_find("vdc1")->first_lba == 63, "vdc1");
	h_true(blk_find("vdc5") && blk_find("vdc5")->first_lba == 1001, "vdc5");
	h_true(blk_find("vdc6") && blk_find("vdc6")->first_lba == 1101, "vdc6");
	h_true(blk_find("vdc6") && blk_find("vdc6")->nsectors == 99, "taille");
	h_true(!blk_find("vdc2") && !blk_find("vdc7"), "ni vdc2 ni vdc7");
}

int	main(void)
{
	h_begin("a11/ebr_scan");
	h_run("ebr/nombre-de-logiques-0-1-max-max+1", nombre_de_logiques);
	h_run("ebr/chaine-interrompue-on-garde-le-lu", chaine_interrompue);
	h_run("ebr/bout-en-bout-noms-des-partitions", bout_en_bout);
	return (h_end());
}
