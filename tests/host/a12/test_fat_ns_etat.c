#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	occupe(void)
{
	t_vfile	*f;
	t_vfile	*g;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_true(vfs_mkdir("/d/m", 0) == 0 && vfs_mkdir("/d/n", 0) == 0
		&& vfs_mkdir("/d/n/sub", 0) == 0, "arbre");
	h_eq_i64("fichier", vst_put("/d/m/f2", "f", 1), 0);
	h_eq_i64("ouvrir", vfs_open("/d/m/f2", O_RDONLY, &f), 0);
	h_eq_i64("supprimer ouvert", vfs_unlink("/d/m/f2"), E_BUSY);
	h_eq_i64("renommer ouvert", vfs_rename("/d/m/f2", "/d/m/f3"), E_BUSY);
	h_eq_i64("deuxième ouverture", vfs_open("/d/M/F2", O_RDWR, &g), 0);
	h_eq_i64("écrire via g", vfs_write(g, "abc", 3), 3);
	h_eq_i64("f voit la taille", vfs_seek(f, 0, SEEK_END_), 3);
	vfs_close(g);
	h_eq_i64("démontage occupé", vfs_umount("/d"), E_BUSY);
	vfs_close(f);
	h_eq_i64("supprimer fermé", vfs_unlink("/d/m/f2"), 0);
	h_eq_i64("ouvrir dossier", vfs_open("/d/n/sub", O_DIRECTORY, &f), 0);
	h_eq_i64("rmdir ouvert", vfs_unlink("/d/n/sub"), E_BUSY);
	vfs_close(f);
	h_eq_i64("rmdir", vfs_unlink("/d/n/sub"), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
}

static void	attribut_ro(void)
{
	t_vfile	*f;
	t_vstat	st;
	uint8_t	*e;

	fakefat_format(g_b, 512, 1);
	fakefat_mount(g_b, "/d");
	vst_put("/d/RO.TXT", "ro", 2);
	vfs_umount("/d");
	e = fake_rootent(g_b, 0);
	h_true(memcmp(e, "RO      TXT", 11) == 0, "entrée attendue");
	e[11] |= 0x01;
	fakefat_mount(g_b, "/d");
	h_eq_i64("écrire attribut RO", vfs_open("/d/RO.TXT", O_WRONLY, &f),
		E_ACCES);
	h_eq_i64("supprimer attribut RO", vfs_unlink("/d/RO.TXT"), E_ACCES);
	h_true(vfs_stat("/d/RO.TXT", &st) == 0 && (st.flags & VFS_STAT_RO),
		"stat RO");
	h_eq_i64("lire RO", vst_same("/d/RO.TXT", "ro", 2), 0);
	vfs_umount("/d");
}

static void	montage_ro(void)
{
	h_eq_i64("montage RO", vfs_mount("/d", "fat32", &g_b->dev,
			VFS_MOUNT_RO), 0);
	h_eq_i64("créer sur montage RO", vst_put("/d/n", "n", 1), E_PERM);
	vfs_umount("/d");
	g_b->dev.flags |= BLK_RO;
	h_eq_i64("montage disque RO", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("mkdir disque RO", vfs_mkdir("/d/k", 0), E_PERM);
	vfs_umount("/d");
	h_eq_u64("aucun accès hors disque", g_b->oob, 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_ns_etat");
	h_run("fichiers occupés", occupe);
	h_run("attribut lecture seule", attribut_ro);
	h_run("montage et disque en lecture seule", montage_ro);
	return (h_end());
}
