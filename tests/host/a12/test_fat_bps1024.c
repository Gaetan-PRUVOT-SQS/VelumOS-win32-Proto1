#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static void	secteur_1024(void)
{
	static uint8_t	data[5000];
	t_fakeblk		*b;

	memset(data, 0x5A, sizeof(data));
	b = fakeblk_new(140000, 512);
	h_eq_i64("format bps 1024", fakefat_format(b, 1024, 1), 0);
	h_eq_i64("montage ratio 2", fakefat_mount(b, "/k"), 0);
	h_eq_i64("écriture", vst_put("/k/Grand.bin", data, 5000), 0);
	vfs_umount("/k");
	h_eq_i64("fsck avant relecture", fake_fsck(b), 0);
	fakefat_mount(b, "/k");
	h_eq_i64("relecture", vst_same("/k/grand.BIN", data, 5000), 0);
	h_eq_i64("démontage", vfs_umount("/k"), 0);
	h_eq_i64("fsck", fake_fsck(b), 0);
	h_eq_u64("aucun accès hors disque", b->oob, 0);
	fakeblk_free(b);
}

int	main(void)
{
	h_begin("a12/fat_bps1024");
	h_run("secteurs FAT de 1024 sur disque 512", secteur_1024);
	return (h_end());
}
