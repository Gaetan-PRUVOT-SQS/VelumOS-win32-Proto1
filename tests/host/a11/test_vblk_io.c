#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "vblk.h"
#include "fake.h"

static uint8_t	*g_buf;

static t_blkdev	*setup(int i, uint64_t sectors, uint64_t feat, uint32_t smax)
{
	t_fkdev		*f;
	uint64_t	k;
	char		name[4];

	f = &g_fkdev[i];
	fk_dev_init(f, sectors, VBLK_PCI_DEVICE);
	f->features |= feat;
	fk_le32(f->devcfg + VBLK_CFG_SIZE_MAX, smax);
	k = 0;
	while (k < f->disk_bytes)
	{
		f->disk[k] = (uint8_t)(k * 7 + k / 512);
		k++;
	}
	if (vblk_probe(&f->pci, (uint32_t)i) < 0)
		return (NULL);
	memcpy(name, "vd", 2);
	name[2] = (char)('a' + i);
	name[3] = '\0';
	return (blk_find(name));
}

static void	io_basic(void)
{
	t_blkdev	*d;
	t_fkdev		*f;

	d = setup(0, 2048, 0, 0);
	f = &g_fkdev[0];
	h_true(d && d->sector_size == 512 && d->nsectors == 2048, "vda 2048x512");
	h_true((f->cmd & 0x406) == 0x406, "memoire, maitre et INTx coupee");
	h_true(f->status == 0x0f, "ACK DRIVER FEATURES_OK DRIVER_OK");
	h_true(f->drv_features == (VIO_F_VERSION_1 | VBLK_F_FLUSH), "fonctions");
	h_true(f->qsize == VQ_SIZE_MAX, "file ramenee a 128");
	h_eq_i64("lecture", blk_read(d, 5, 3, g_buf), 0);
	h_true(!memcmp(g_buf, f->disk + 5 * 512, 3 * 512), "donnees lues");
	h_eq_u64("secteur demande", f->last_sector, 5);
	memset(g_buf, 0x3c, 1024);
	h_eq_i64("ecriture", blk_write(d, 2046, 2, g_buf), 0);
	h_true(f->disk[2046 * 512] == 0x3c && f->disk[2048 * 512 - 1] == 0x3c,
		"donnees ecrites en fin de disque");
	h_eq_i64("vidage", blk_flush(d), 0);
	h_true(f->flushes == 1 && f->last_type == VBLK_T_FLUSH, "flush envoye");
	h_eq_i64("verrous rendus", g_fk.locks, 0);
}

static void	io_chunks(void)
{
	t_blkdev	*d;

	d = setup(1, 4096, VBLK_F_SIZE_MAX, 4096);
	h_eq_i64("lecture de 300 secteurs", blk_read(d, 100, 300, g_buf), 0);
	h_eq_i64("38 requetes de 8 secteurs", g_fkdev[1].requests, 38);
	h_true(!memcmp(g_buf, g_fkdev[1].disk + 100 * 512, 300 * 512),
		"morceaux recolles");
	memset(g_buf, 0x5a, 300 * 512);
	h_eq_i64("ecriture de 300 secteurs", blk_write(d, 7, 300, g_buf), 0);
	h_true(!memcmp(g_buf, g_fkdev[1].disk + 7 * 512, 300 * 512),
		"ecriture recollee");
	d = setup(2, 4096, 0, 0);
	h_eq_i64("sans size_max", blk_read(d, 0, 300, g_buf), 0);
	h_eq_i64("3 requetes de 64 Kio au plus", g_fkdev[2].requests, 3);
	fk_dev_init(&g_fkdev[3], 64, VBLK_PCI_DEVICE);
	g_fkdev[3].features |= VBLK_F_SIZE_MAX;
	fk_le32(g_fkdev[3].devcfg + VBLK_CFG_SIZE_MAX, 511);
	h_eq_i64("size_max sous un secteur",
		vblk_probe(&g_fkdev[3].pci, 3), E_NOTSUP);
}

static void	io_blk_size_ro(void)
{
	t_blkdev	*d;

	fk_dev_free(&g_fkdev[3]);
	fk_dev_init(&g_fkdev[3], 2048, VBLK_PCI_DEVICE);
	g_fkdev[3].features = VIO_F_VERSION_1 | VBLK_F_BLK_SIZE | VBLK_F_RO;
	fk_le32(g_fkdev[3].devcfg + VBLK_CFG_BLK_SIZE, 4096);
	h_eq_i64("sonde 4096", vblk_probe(&g_fkdev[3].pci, 3), 0);
	d = blk_find("vdd");
	h_true(d && d->sector_size == 4096 && d->nsectors == 256, "256 x 4096");
	h_true(d && (d->flags & BLK_RO), "lecture seule annoncee");
	h_eq_i64("lecture lba 3", blk_read(d, 3, 1, g_buf), 0);
	h_eq_u64("secteur en unites de 512", g_fkdev[3].last_sector, 24);
	h_eq_i64("ecriture refusee", blk_write(d, 0, 1, g_buf), E_PERM);
	h_eq_i64("une seule requete", g_fkdev[3].requests, 1);
	h_eq_i64("vidage sans F_FLUSH", blk_flush(d), 0);
	h_eq_i64("aucune requete de vidage", g_fkdev[3].requests, 1);
}

int	main(void)
{
	g_buf = malloc(300 * 512);
	if (!g_buf)
		return (1);
	h_begin("a11/vblk_io");
	h_run("lecture ecriture vidage", io_basic);
	h_run("decoupage en morceaux", io_chunks);
	h_run("blk_size et lecture seule", io_blk_size_ro);
	free(g_buf);
	fk_dev_free(&g_fkdev[0]);
	fk_dev_free(&g_fkdev[1]);
	fk_dev_free(&g_fkdev[2]);
	fk_dev_free(&g_fkdev[3]);
	return (h_end());
}
