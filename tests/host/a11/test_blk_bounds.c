#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram	g_disk;
static t_blkdev	g_part;
static uint8_t	g_buf[2 * 512];

static void	bounds_limits(void)
{
	t_blkdev	*d;

	d = &g_disk.dev;
	h_eq_i64("dernier secteur", blk_read(d, 99, 1, g_buf), 0);
	h_eq_i64("lba = fin", blk_read(d, 100, 1, g_buf), E_RANGE);
	h_eq_i64("deborde d'un", blk_read(d, 99, 2, g_buf), E_RANGE);
	h_eq_i64("lba maximal", blk_read(d, UINT64_MAX, 1, g_buf), E_RANGE);
	h_eq_i64("n maximal", blk_read(d, 1, UINT32_MAX, g_buf), E_RANGE);
	h_eq_i64("n nul", blk_read(d, 0, 0, g_buf), E_INVAL);
	h_eq_i64("tampon nul", blk_read(d, 0, 1, NULL), E_INVAL);
	h_eq_i64("disque nul", blk_read(NULL, 0, 1, g_buf), E_INVAL);
	h_eq_i64("ecriture hors disque", blk_write(d, 100, 1, g_buf), E_RANGE);
	h_eq_i64("ecriture tampon nul", blk_write(d, 0, 1, NULL), E_INVAL);
	h_eq_i64("ecriture disque nul", blk_write(NULL, 0, 1, g_buf), E_INVAL);
	h_eq_i64("flush nul", blk_flush(NULL), E_INVAL);
	h_eq_i64("pilote appele une fois", g_disk.reads, 1);
	h_eq_i64("pilote jamais en ecriture", g_disk.writes, 0);
}

static void	bounds_mcdc(void)
{
	t_blkdev	d;

	d.nsectors = 10;
	h_eq_i64("A vrai", blk_range_ok(&d, 0, 0), E_INVAL);
	h_eq_i64("A faux B vrai", blk_range_ok(&d, 10, 1), E_RANGE);
	h_eq_i64("A faux B faux C vrai", blk_range_ok(&d, 9, 2), E_RANGE);
	h_eq_i64("toutes fausses", blk_range_ok(&d, 9, 1), 0);
	d.nsectors = UINT64_MAX;
	h_eq_i64("disque maximal, fin exacte",
		blk_range_ok(&d, UINT64_MAX - 5, 5), 0);
	h_eq_i64("disque maximal, un de trop",
		blk_range_ok(&d, UINT64_MAX - 5, 6), E_RANGE);
}

static void	ro_decision(void)
{
	g_disk.dev.flags = 0;
	g_part.flags = BLK_RO;
	h_eq_i64("partition RO", blk_write(&g_part, 0, 1, g_buf), E_PERM);
	h_eq_i64("disque rw", blk_write(&g_disk.dev, 0, 1, g_buf), 0);
	g_disk.dev.flags = BLK_RO;
	g_part.flags = 0;
	h_eq_i64("parent RO", blk_write(&g_part, 0, 1, g_buf), E_PERM);
	h_eq_i64("disque RO", blk_write(&g_disk.dev, 0, 1, g_buf), E_PERM);
	h_eq_i64("lecture sur RO", blk_read(&g_part, 0, 1, g_buf), 0);
	h_eq_i64("vidage sur RO", blk_flush(&g_part), 0);
	g_disk.dev.flags = 0;
	h_eq_i64("aucun RO", blk_write(&g_part, 0, 1, g_buf), 0);
}

static void	part_translation(void)
{
	h_eq_i64("lecture 0", blk_read(&g_part, 0, 1, g_buf), 0);
	h_eq_u64("lba translate", g_disk.last_lba, 10);
	h_eq_i64("lecture fin", blk_read(&g_part, 49, 1, g_buf), 0);
	h_eq_u64("lba fin translate", g_disk.last_lba, 59);
	h_eq_i64("hors partition", blk_read(&g_part, 50, 1, g_buf), E_RANGE);
	memset(g_buf, 0x77, sizeof(g_buf));
	h_eq_i64("ecriture", blk_write(&g_part, 5, 2, g_buf), 0);
	h_eq_u64("ecriture translatee", g_disk.last_lba, 15);
	h_true(g_disk.data[15 * 512] == 0x77 && g_disk.data[17 * 512 - 1] == 0x77,
		"donnees au bon endroit");
	h_eq_i64("vidage", blk_flush(&g_part), 0);
	h_eq_i64("vidage du parent", g_disk.flushes, 2);
	g_disk.fail_rc = E_IO;
	h_eq_i64("erreur du pilote propagee", blk_read(&g_part, 0, 1, g_buf),
		E_IO);
	g_disk.fail_rc = 0;
}

int	main(void)
{
	fk_ram_init(&g_disk, "vda", 512, 100);
	blk_register(&g_disk.dev);
	memcpy(g_part.name, "vda1", 5);
	g_part.sector_size = 512;
	g_part.nsectors = 50;
	g_part.first_lba = 10;
	g_part.parent = &g_disk.dev;
	h_begin("a11/bornes");
	h_eq_i64("partition enregistree", blk_register(&g_part), 0);
	h_run("valeurs limites", bounds_limits);
	h_run("MC/DC blk_range_ok", bounds_mcdc);
	h_run("lecture seule", ro_decision);
	h_run("translation de partition", part_translation);
	fk_ram_free(&g_disk);
	return (h_end());
}
