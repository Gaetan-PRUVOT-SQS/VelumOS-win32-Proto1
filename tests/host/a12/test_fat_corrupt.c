#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static uint32_t	first_of(uint32_t idx)
{
	uint8_t	*e;

	e = fake_rootent(g_b, idx);
	return ((le16(e + 20) << 16) | le16(e + 26));
}

static void	prepare(void)
{
	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	fakefat_mount(g_b, "/d");
	vst_put("/d/BOUCLE.BIN", g_b->img, 1500);
	vst_put("/d/CASSE.BIN", g_b->img, 1500);
	vst_put("/d/TAILLE.BIN", g_b->img, 100);
	vfs_umount("/d");
	fakefat_set(g_b, first_of(0) + 2, first_of(0));
	fakefat_set(g_b, first_of(1) + 1, 0);
	put32(fake_rootent(g_b, 2) + 28, 5000);
}

static void	chaines(void)
{
	t_vfile	*f;
	t_vstat	st;

	prepare();
	fakefat_mount(g_b, "/d");
	h_eq_i64("chaîne bouclante", vfs_open("/d/BOUCLE.BIN", O_RDONLY, &f),
		E_IO);
	h_eq_i64("chaîne cassée", vfs_open("/d/CASSE.BIN", O_RDONLY, &f), E_IO);
	h_eq_i64("taille > chaîne", vfs_open("/d/TAILLE.BIN", O_RDONLY, &f), E_IO);
	h_eq_i64("stat reste possible", vfs_stat("/d/BOUCLE.BIN", &st), 0);
	vfs_umount("/d");
}

static void	hors_volume(void)
{
	t_vfile	*f;
	t_vstat	st;
	uint8_t	*e;

	e = fake_rootent(g_b, 0);
	put16(e + 20, 0x0FFF);
	put16(e + 26, 0xFFF0);
	fakefat_mount(g_b, "/d");
	h_eq_i64("premier cluster hors volume", vfs_stat("/d/BOUCLE.BIN", &st),
		E_IO);
	vfs_umount("/d");
	fakefat_set(g_b, 2, 2);
	fakefat_mount(g_b, "/d");
	h_eq_i64("racine bouclante : ouverture", vfs_open("/d", O_DIRECTORY, &f),
		E_IO);
	h_eq_i64("racine bouclante : recherche bornée", vfs_stat("/d/ABSENT",
			&st), E_NOENT);
	vfs_umount("/d");
	h_eq_u64("aucun accès hors disque", g_b->oob, 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_corrupt");
	h_run("chaînes bouclantes, cassées, trop courtes", chaines);
	h_run("premier cluster hors volume, racine bouclante", hors_volume);
	return (h_end());
}
