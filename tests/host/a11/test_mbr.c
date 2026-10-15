#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static uint8_t		g_sec[512];
static t_partlist	g_pl;

static void	mbr_valid(void)
{
	static const t_fkpart	p[3] = {{63, 1062, 0}, {2048, 6143, 1},
	{7000, 7999, 3}};

	memset(g_sec, 0, sizeof(g_sec));
	fk_mbr_entry(g_sec, 0, 0x83, &p[0]);
	fk_mbr_entry(g_sec, 1, 0x0c, &p[1]);
	fk_mbr_entry(g_sec, 3, 0x05, &p[2]);
	g_sec[446] = 0x80;
	h_eq_i64("analyse", mbr_parse(g_sec, 10000, &g_pl), 0);
	h_eq_u64("deux primaires", g_pl.n, 2);
	h_eq_u64("etendue comptee", g_pl.extended, 1);
	h_true(!g_pl.gpt, "pas de gpt");
	h_eq_u64("p1 debut", g_pl.p[0].first, 63);
	h_eq_u64("p1 taille", g_pl.p[0].count, 1000);
	h_eq_u64("p1 numero", g_pl.p[0].num, 1);
	h_eq_u64("p2 numero", g_pl.p[1].num, 2);
	g_sec[446 + 16 * 3 + 4] = 0xee;
	h_eq_i64("protectrice", mbr_parse(g_sec, 10000, &g_pl), 0);
	h_true(g_pl.gpt && g_pl.n == 0, "gpt annonce");
}

static void	mbr_not_table(void)
{
	h_eq_i64("analyse", mbr_parse(g_sec, 10000, &g_pl), 0);
	g_sec[511] = 0;
	h_eq_i64("sans signature", mbr_parse(g_sec, 10000, &g_pl), 0);
	h_true(g_pl.n == 0 && !g_pl.gpt, "rien sans signature");
	g_sec[511] = 0xaa;
	g_sec[446 + 16] = 0x12;
	h_eq_i64("amorce invalide", mbr_parse(g_sec, 10000, &g_pl), 0);
	h_true(g_pl.n == 0 && !g_pl.gpt, "secteur de volume, pas une table");
	memset(g_sec, 0, sizeof(g_sec));
	g_sec[510] = 0x55;
	g_sec[511] = 0xaa;
	h_eq_i64("table vide", mbr_parse(g_sec, 10000, &g_pl), 0);
	h_eq_u64("aucune partition", g_pl.n, 0);
}

static void	mbr_rejects(void)
{
	static const t_fkpart	p[6] = {{10, 99, 0}, {50, 150, 0}, {0, 9, 0},
	{9990, 10000, 0}, {10000, 10000, 0}, {9990, 9999, 0}};

	memset(g_sec, 0, sizeof(g_sec));
	fk_mbr_entry(g_sec, 0, 0x83, &p[0]);
	fk_mbr_entry(g_sec, 1, 0x83, &p[1]);
	h_eq_i64("chevauchement", mbr_parse(g_sec, 10000, &g_pl), E_PROTO);
	h_eq_u64("liste videe", g_pl.n, 0);
	fk_mbr_entry(g_sec, 1, 0x83, &p[2]);
	h_eq_i64("lba 0", mbr_parse(g_sec, 10000, &g_pl), E_PROTO);
	fk_mbr_entry(g_sec, 1, 0x83, &p[3]);
	h_eq_i64("deborde d'un", mbr_parse(g_sec, 10000, &g_pl), E_PROTO);
	fk_mbr_entry(g_sec, 1, 0x83, &p[4]);
	h_eq_i64("debut hors disque", mbr_parse(g_sec, 10000, &g_pl), E_PROTO);
	fk_mbr_entry(g_sec, 1, 0x83, &p[5]);
	h_eq_i64("fin exacte", mbr_parse(g_sec, 10000, &g_pl), 0);
	g_sec[446 + 16 + 12] = 0;
	g_sec[446 + 16 + 13] = 0;
	h_eq_i64("taille nulle ignoree", mbr_parse(g_sec, 10000, &g_pl), 0);
	h_eq_u64("une seule", g_pl.n, 1);
}

static void	mbr_scan(void)
{
	static t_fkram	r;
	static t_fkram	other;
	int				live;

	fk_ram_init(&r, "vdb", 512, 10000);
	fk_ram_init(&other, "vdc", 512, 100);
	memcpy(r.data, g_sec, 512);
	blk_register(&r.dev);
	live = fk_live();
	fk_fail_after(0);
	h_eq_i64("echec allocation 1", blk_scan_partitions(&r.dev), E_NOMEM);
	fk_fail_after(1);
	h_eq_i64("echec allocation 2", blk_scan_partitions(&r.dev), E_NOMEM);
	fk_fail_after(-1);
	h_eq_i64("rien de garde", fk_live(), live);
	h_eq_i64("scan", blk_scan_partitions(&r.dev), 1);
	h_true(blk_find("vdb1") && blk_find("vdb1")->first_lba == 10, "vdb1");
	h_eq_i64("deuxieme scan", blk_scan_partitions(&r.dev), E_EXIST);
	h_eq_i64("scan d'une partition", blk_scan_partitions(blk_find("vdb1")),
		E_INVAL);
	h_eq_i64("disque inconnu", blk_scan_partitions(&other.dev), E_INVAL);
	r.fail_rc = E_IO;
	h_eq_i64("erreur de lecture", blk_scan_partitions(&r.dev), E_IO);
	fk_ram_free(&r);
	fk_ram_free(&other);
}

int	main(void)
{
	h_begin("a11/mbr");
	h_run("table valide", mbr_valid);
	h_run("pas une table", mbr_not_table);
	h_run("tables refusees", mbr_rejects);
	h_run("scan et noms", mbr_scan);
	return (h_end());
}
