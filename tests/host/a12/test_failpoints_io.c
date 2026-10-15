#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fsys.h"
#include "velum/err.h"
#include "velum/heap.h"

static t_fakeblk	*g_b;

static void	appels_sous_pression(void)
{
	char	buf[16];
	int64_t	h;
	int64_t	r;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("fichier", vst_put("/d/Renommé sous pression.txt", "x", 1), 0);
	h = sys_open(fake_args((uint64_t)"/d/Renommé sous pression.txt", 29,
				O_RDONLY, 0));
	h_true(h > 0, "ouverture");
	heap_fail_after(0);
	r = sys_read(fake_args(h, (uint64_t)buf, 16, 0));
	heap_fail_after(-1);
	h_eq_i64("read sans tampon noyau", r, E_NOMEM);
	fake_close((t_handle)h);
	heap_fail_after(0);
	h = sys_open(fake_args((uint64_t)"/d/Renommé sous pression.txt", 29,
				O_RDONLY, 0));
	heap_fail_after(-1);
	h_eq_i64("open sans mémoire", h, E_NOMEM);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck après pression mémoire", fake_fsck(g_b), 0);
}

static void	disque_defaillant(void)
{
	char	path[64];
	int64_t	k;
	int		rc;

	k = 0;
	while (k < 40)
	{
		fakefat_mount(g_b, "/d");
		snprintf(path, sizeof(path), "/d/Échec disque %lld.bin",
			(long long)k);
		g_b->wfail = k;
		rc = vst_put(path, g_b->img, 3000);
		g_b->wfail = -1;
		h_true(rc == 0 || rc == E_IO, "écriture disque : E_IO propre");
		g_b->rfail = k % 7;
		fake_walk("/d");
		g_b->rfail = -1;
		h_eq_i64("démontage après panne", vfs_umount("/d"), 0);
		k++;
	}
	h_eq_u64("aucun accès hors disque", g_b->oob, 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/failpoints_io");
	h_run("appels sous pression mémoire", appels_sous_pression);
	h_run("disque défaillant en écriture et lecture", disque_defaillant);
	return (h_end());
}
