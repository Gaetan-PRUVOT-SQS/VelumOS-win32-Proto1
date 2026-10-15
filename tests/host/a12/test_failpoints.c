#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fsys.h"
#include "velum/err.h"
#include "velum/heap.h"

static t_fakeblk	*g_b;
static char			g_path[64];

static void	montage_fat(void)
{
	uint64_t	live;
	int64_t		k;
	int			rc;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	live = fake_heap_live();
	k = 0;
	rc = E_NOMEM;
	while (rc == E_NOMEM && k < 50)
	{
		heap_fail_after(k++);
		rc = fakefat_mount(g_b, "/d");
		heap_fail_after(-1);
		h_true(rc == 0 || fake_heap_live() == live, "montage FAT sans fuite");
	}
	h_eq_i64("montage FAT finit par réussir", rc, 0);
}

static void	montage_initrd(void)
{
	uint8_t		raw[4096];
	t_vbuf		b;
	int64_t		k;
	int			rc;

	b.p = raw;
	b.len = 0;
	b.cap = sizeof(raw);
	fakecpio_valid(&b);
	rc = E_NOMEM;
	k = 0;
	while (rc == E_NOMEM && k < 50)
	{
		heap_fail_after(k++);
		rc = vfs_mount_initrd("/r", raw, b.len);
		heap_fail_after(-1);
	}
	h_eq_i64("montage initrd finit par réussir", rc, 0);
	h_eq_i64("démontage initrd", vfs_umount("/r"), 0);
}

static void	ecriture_chaque_rang(void)
{
	uint64_t	live;
	int64_t		k;
	int			rc;

	live = fake_heap_live();
	k = 0;
	rc = E_NOMEM;
	while (rc != 0 && k < 60)
	{
		snprintf(g_path, sizeof(g_path), "/d/rang %lld.txt", (long long)k);
		heap_fail_after(k++);
		rc = vst_put(g_path, g_b->img, 2000);
		heap_fail_after(-1);
		h_true(rc == 0 || rc == E_NOMEM, "échec propre E_NOMEM");
		h_eq_u64("tas revenu au niveau", fake_heap_live(), live);
	}
	h_eq_i64("écriture finit par réussir", rc, 0);
}

static void	renommage_chaque_rang(void)
{
	int64_t	k;
	int		rc;

	k = 0;
	rc = E_NOMEM;
	while (rc != 0 && k < 60)
	{
		heap_fail_after(k++);
		rc = vfs_rename(g_path, "/d/Renommé sous pression.txt");
		heap_fail_after(-1);
	}
	h_eq_i64("renommage finit par réussir", rc, 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck après pression mémoire", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/failpoints");
	h_run("montage FAT : allocation qui échoue à chaque rang", montage_fat);
	h_run("montage initrd : allocation qui échoue à chaque rang",
		montage_initrd);
	h_run("écriture : allocation qui échoue à chaque rang",
		ecriture_chaque_rang);
	h_run("renommage : allocation qui échoue à chaque rang",
		renommage_chaque_rang);
	return (h_end());
}
