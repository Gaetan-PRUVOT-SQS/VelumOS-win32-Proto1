#include <stdlib.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	disque_plein(void)
{
	t_vfile		*f;
	uint8_t		*big;
	int64_t		w;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	fakefat_bad_from(g_b, 23);
	h_eq_i64("montage presque plein", fakefat_mount(g_b, "/d"), 0);
	big = calloc(30, 512);
	h_eq_i64("créer", vfs_open("/d/plein.bin", O_CREAT | O_WRONLY, &f), 0);
	w = vfs_write(f, big, 30 * 512);
	h_eq_i64("écriture partielle", w, 20 * 512);
	h_eq_i64("ensuite E_NOSPC", vfs_write(f, big, 512), E_NOSPC);
	vfs_close(f);
	h_eq_i64("mkdir plein", vfs_mkdir("/d/dossier", 0), E_NOSPC);
	h_eq_i64("supprimer", vfs_unlink("/d/plein.bin"), 0);
	h_eq_i64("mkdir après libération", vfs_mkdir("/d/dossier", 0), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck disque plein", fake_fsck(g_b), 0);
	h_eq_u64("aucun accès hors disque", g_b->oob, 0);
	free(big);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_plein");
	h_run("disque plein", disque_plein);
	return (h_end());
}
