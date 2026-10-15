#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "vblk.h"
#include "fake.h"

static uint8_t	*g_ref;

static void	init_empty(void)
{
	h_eq_i64("init sans disque", block_boot_init(), 0);
	h_eq_u64("aucun peripherique", blk_count(), 0);
	h_eq_i64("autotest sans disque", block_selftest(), 0);
	snprintf(g_fk.cmdline, sizeof(g_fk.cmdline), "selftest blktest");
	h_eq_i64("blktest sans partition de test", block_selftest(), 1);
	g_fk.cmdline[0] = '\0';
}

static void	prepare(void)
{
	static const t_fkpart	p[1] = {{2048, 4095, 0}};
	static const t_fkpart	m[1] = {{63, 1062, 0}};
	static t_fkram			img;

	fk_dev_init(&g_fkdev[0], 8192, VBLK_PCI_DEVICE);
	fk_ram_init(&img, "img", 512, 8192);
	fk_gpt_build(&img, p, 1);
	fk_pattern(img.data + 2048 * 512, 2048);
	memcpy(g_fkdev[0].disk, img.data, 8192 * 512);
	memcpy(g_ref, img.data + 2048 * 512, 2048 * 512);
	fk_ram_free(&img);
	fk_dev_init(&g_fkdev[1], 64, VBLK_PCI_LEGACY);
	fk_dev_init(&g_fkdev[2], 2048, VBLK_PCI_DEVICE);
	g_fkdev[2].features |= VBLK_F_RO;
	fk_mbr_entry(g_fkdev[2].disk, 0, 0x83, &m[0]);
}

static void	init_disks(void)
{
	prepare();
	fk_reset_log();
	h_eq_i64("init", block_boot_init(), 0);
	h_true(blk_find("vda") && blk_find("vda1") && blk_find("vdb")
		&& blk_find("vdb1"), "disques et partitions");
	h_eq_u64("quatre peripheriques", blk_count(), 4);
	h_true(blk_find("vdb1") && (blk_find("vdb1")->flags & BLK_RO),
		"lecture seule heritee");
	h_true(g_fk.warns >= 1, "legacy journalise");
	h_eq_i64("autotest de lecture", block_selftest(), 0);
}

static void	selftest_write(void)
{
	uint8_t	*part;
	int		live;

	part = g_fkdev[0].disk + 2048 * 512;
	snprintf(g_fk.cmdline, sizeof(g_fk.cmdline), "blktest");
	g_fkdev[0].flushes = 0;
	h_eq_i64("autotest d'ecriture", block_selftest(), 0);
	h_true(!memcmp(part, g_ref, 2048 * 512), "motif restaure");
	h_eq_i64("deux vidages", g_fkdev[0].flushes, 2);
	part[5 * 512 + 7] ^= 1;
	h_eq_i64("motif abime detecte", block_selftest(), 1);
	part[5 * 512 + 7] ^= 1;
	live = fk_live();
	fk_fail_after(0);
	h_true(block_selftest() == 5, "allocation refusee : 4 + 1 echecs");
	fk_fail_after(-1);
	h_eq_i64("rien garde", fk_live(), live);
	h_eq_i64("verrous rendus", g_fk.locks, 0);
}

int	main(void)
{
	g_ref = malloc(2048 * 512);
	if (!g_ref)
		return (1);
	h_begin("a11/block_init");
	h_run("init sans disque", init_empty);
	h_run("init avec disques", init_disks);
	h_run("autotest d'ecriture", selftest_write);
	free(g_ref);
	fk_dev_free(&g_fkdev[0]);
	fk_dev_free(&g_fkdev[1]);
	fk_dev_free(&g_fkdev[2]);
	return (h_end());
}
