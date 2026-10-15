#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static uint8_t	g_raw[4096];

static void	ecriture_refusee(void)
{
	t_vfile	*f;
	t_vbuf	b;

	b.p = g_raw;
	b.len = 0;
	b.cap = sizeof(g_raw);
	fakecpio_valid(&b);
	h_eq_i64("montage", vfs_mount_initrd("/r", g_raw, b.len), 0);
	h_eq_i64("O_WRONLY", vfs_open("/r/vide", O_WRONLY, &f), E_PERM);
	h_eq_i64("O_RDWR", vfs_open("/r/vide", O_RDWR, &f), E_PERM);
	h_eq_i64("O_CREAT", vfs_open("/r/neuf", O_CREAT | O_WRONLY, &f), E_PERM);
	h_eq_i64("O_TRUNC", vfs_open("/r/vide", O_TRUNC | O_RDWR, &f), E_PERM);
	h_eq_i64("O_TRUNC lecture", vfs_open("/r/vide", O_TRUNC, &f), E_INVAL);
	h_eq_i64("O_EXCL seul", vfs_open("/r/vide", O_EXCL, &f), E_INVAL);
	h_eq_i64("accès 3", vfs_open("/r/vide", 3, &f), E_INVAL);
	h_eq_i64("bit inconnu", vfs_open("/r/vide", 0x8, &f), E_INVAL);
	h_eq_i64("mkdir", vfs_mkdir("/r/d", 0), E_PERM);
	h_eq_i64("unlink", vfs_unlink("/r/vide"), E_PERM);
	h_eq_i64("rename", vfs_rename("/r/vide", "/r/v2"), E_PERM);
	h_eq_i64("ouvrir", vfs_open("/r/vide", O_RDONLY, &f), 0);
	h_eq_i64("écrire handle lecture", vfs_write(f, "a", 1), E_BADF);
	h_eq_i64("tronquer handle lecture", vfs_truncate(f, 0), E_BADF);
	vfs_close(f);
	h_eq_i64("démontage", vfs_umount("/r"), 0);
}

int	main(void)
{
	h_begin("a12/vfs_initrd_ro");
	h_run("écriture refusée (lecture seule)", ecriture_refusee);
	return (h_end());
}
