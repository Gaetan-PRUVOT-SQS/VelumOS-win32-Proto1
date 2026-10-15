#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static const t_fkpart	g_parts[2] = {{2048, 4095, 0}, {4096, 6000, 2}};

static t_fkram	*disk(const char *name, uint32_t ss, uint64_t ns)
{
	static t_fkram	r[8];
	static int		n;
	t_fkram			*d;

	d = &r[n++];
	fk_ram_init(d, name, ss, ns);
	fk_gpt_build(d, g_parts, 2);
	return (d);
}

static void	gpt_valid(void)
{
	t_fkram		*d;
	t_blkdev	*p;

	d = disk("vda", 512, 8192);
	blk_register(&d->dev);
	fk_reset_log();
	h_eq_i64("scan", blk_scan_partitions(&d->dev), 2);
	p = blk_find("vda1");
	h_true(p && p->first_lba == 2048 && p->nsectors == 2048, "vda1");
	p = blk_find("vda3");
	h_true(p && p->first_lba == 4096 && p->nsectors == 1905, "vda3 inclusif");
	h_true(blk_find("vda2") == NULL, "entree vide ignoree");
	h_eq_i64("aucun avertissement", g_fk.warns, 0);
	d = disk("vdk", 4096, 8192);
	blk_register(&d->dev);
	h_eq_i64("secteurs de 4096", blk_scan_partitions(&d->dev), 2);
	h_true(blk_find("vdk3") && blk_find("vdk3")->sector_size == 4096,
		"partition en 4096");
}

static void	gpt_backup(void)
{
	static t_fkram	tiny;
	t_fkram			*d;

	d = disk("vdb", 512, 8192);
	blk_register(&d->dev);
	d->data[512 + 60] ^= 1;
	fk_reset_log();
	h_eq_i64("principale abimee : copie", blk_scan_partitions(&d->dev), 2);
	h_eq_i64("deux avertissements", g_fk.warns, 2);
	h_true(blk_find("vdb1") != NULL, "vdb1 depuis la copie");
	d = disk("vdc", 512, 8192);
	blk_register(&d->dev);
	d->data[512 + 60] ^= 1;
	d->data[8191 * 512 + 60] ^= 1;
	h_eq_i64("deux en-tetes abimes", blk_scan_partitions(&d->dev), E_PROTO);
	h_true(blk_find("vdc1") == NULL, "rien enregistre");
	fk_ram_init(&tiny, "vdf", 512, 2);
	fk_mbr_entry(tiny.data, 0, MBR_TYPE_GPT, &g_parts[0]);
	blk_register(&tiny.dev);
	h_eq_i64("disque trop petit", blk_scan_partitions(&tiny.dev), E_INVAL);
}

static void	gpt_backup_only(void)
{
	t_fkram	*d;

	d = disk("vdd", 512, 8192);
	blk_register(&d->dev);
	d->data[8191 * 512 + 60] ^= 1;
	fk_reset_log();
	h_eq_i64("copie abimee seule", blk_scan_partitions(&d->dev), 2);
	h_eq_i64("un avertissement", g_fk.warns, 1);
	d = disk("vde", 512, 8192);
	blk_register(&d->dev);
	d->data[2 * 512 + 100] ^= 1;
	fk_reset_log();
	h_eq_i64("tableau principal abime : copie",
		blk_scan_partitions(&d->dev), 2);
	h_eq_i64("avertis", g_fk.warns, 2);
}

int	main(void)
{
	h_begin("a11/gpt");
	h_run("table valide", gpt_valid);
	h_run("copie de secours", gpt_backup);
	h_run("copie seule abimee", gpt_backup_only);
	h_eq_i64("verrous rendus", g_fk.locks, 0);
	h_eq_i64("seules les partitions restent allouees", fk_live(),
		(int)blk_count() - 7);
	return (h_end());
}
