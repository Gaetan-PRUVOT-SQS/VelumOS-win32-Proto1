#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static int	pattern_ok(const char *path, uint32_t len, uint32_t seed, int mk)
{
	uint8_t		*want;
	uint32_t	i;
	int			rc;

	want = malloc(len + 1);
	i = 0;
	while (i < len)
	{
		want[i] = (uint8_t)(i * 31 + seed + i / 512);
		i++;
	}
	if (mk)
		rc = vst_put(path, want, len);
	else
		rc = vst_same(path, want, len);
	free(want);
	return (rc);
}

static void	tailles_limites(void)
{
	static const uint32_t	sizes[] = {0, 1, 511, 512, 513, 1024, 4097,
		300000, 0xFFFFFFFF};
	char					path[64];
	uint32_t				i;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	i = 0;
	while (sizes[i] != 0xFFFFFFFF)
	{
		snprintf(path, sizeof(path), "/d/F%u.BIN", sizes[i]);
		h_eq_i64("écriture taille limite", pattern_ok(path, sizes[i], i, 1),
			0);
		h_eq_i64("relecture", pattern_ok(path, sizes[i], i, 0), 0);
		i++;
	}
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck après démontage", fake_fsck(g_b), 0);
	h_eq_i64("remontage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("relecture gros après remontage",
		pattern_ok("/d/F300000.BIN", 300000, 7, 0), 0);
	h_eq_i64("relecture sans casse", pattern_ok("/d/f513.bin", 513, 4, 0),
		0);
}

static void	trous_et_troncature(void)
{
	t_vfile		*f;
	uint8_t		buf[6000];
	t_vstat		st;

	h_eq_i64("créer", vfs_open("/d/trou.bin", O_CREAT | O_RDWR, &f), 0);
	h_eq_i64("écrire à 5000", vfs_pwrite(f, "Z", 1, 5000), 1);
	h_true(vfs_fstat(f, &st) == 0 && st.size == 5001, "taille avec trou");
	memset(buf, 1, sizeof(buf));
	h_eq_i64("relire trou", vfs_pread(f, buf, 6000, 0), 5001);
	h_true(buf[0] == 0 && buf[4999] == 0 && buf[5000] == 'Z', "trou à zéro");
	h_eq_i64("tronquer 100", vfs_truncate(f, 100), 0);
	h_eq_i64("pwrite garde", vfs_pwrite(f, "AB", 2, 98), 2);
	h_eq_i64("agrandir 3000", vfs_truncate(f, 3000), 0);
	h_true(vfs_pread(f, buf, 6000, 0) == 3000 && buf[99] == 'B'
		&& buf[100] == 0 && buf[2999] == 0, "agrandissement à zéro");
	h_eq_i64("tronquer 0", vfs_truncate(f, 0), 0);
	h_eq_i64("tronquer > 4 Gio", vfs_truncate(f, 1ull << 32), E_RANGE);
	h_eq_i64("écrire à 4 Gio - 1", vfs_pwrite(f, "x", 1, 0xFFFFFFFFull),
		E_RANGE);
	vfs_close(f);
}

static void	ajout_et_vidage(void)
{
	t_vfile		*f;
	t_vstat		st;

	h_eq_i64("ajout", vfs_open("/d/trou.bin", O_WRONLY | O_APPEND, &f), 0);
	h_true(vfs_write(f, "12", 2) == 2 && vfs_write(f, "34", 2) == 2,
		"O_APPEND");
	vfs_close(f);
	h_eq_i64("O_APPEND contenu", vst_same("/d/trou.bin", "1234", 4), 0);
	h_eq_i64("O_TRUNC", vfs_open("/d/trou.bin", O_WRONLY | O_TRUNC, &f), 0);
	h_true(vfs_fstat(f, &st) == 0 && st.size == 0, "O_TRUNC vide");
	vfs_close(f);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	h_eq_u64("aucun accès hors disque", g_b->oob, 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_rw");
	h_run("tailles limites et remontage", tailles_limites);
	h_run("trous, troncature", trous_et_troncature);
	h_run("ajout, O_TRUNC", ajout_et_vidage);
	return (h_end());
}
