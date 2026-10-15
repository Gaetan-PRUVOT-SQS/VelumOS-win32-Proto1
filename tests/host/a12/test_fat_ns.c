#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	dossiers(void)
{
	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_true(vfs_mkdir("/d/a", 0) == 0 && vfs_mkdir("/d/a/b", 0) == 0, "mkdir");
	h_eq_i64("mkdir existant", vfs_mkdir("/d/a", 0), E_EXIST);
	h_eq_i64("mkdir parent absent", vfs_mkdir("/d/x/y", 0), E_NOENT);
	h_eq_i64("rmdir non vide", vfs_unlink("/d/a"), E_NOTEMPTY);
	h_eq_i64("rmdir b", vfs_unlink("/d/a/b"), 0);
	h_eq_i64("rmdir a", vfs_unlink("/d/a"), 0);
}

static void	dossier_etendu(void)
{
	char		p[64];
	t_vfile		*f;
	t_dirent	d;
	int			i;

	h_eq_i64("mkdir big", vfs_mkdir("/d/big", 0), 0);
	i = -1;
	while (++i < 40)
	{
		snprintf(p, sizeof(p), "/d/big/Fichier numéro %02d.dat", i);
		vst_put(p, p, 4);
	}
	vfs_open("/d/big", O_DIRECTORY, &f);
	i = 0;
	while (vfs_readdir(f, &d) == 1)
		i++;
	vfs_close(f);
	h_eq_i64("40 entrées sur plusieurs clusters", i, 40);
	h_eq_i64("fichier comme parent", vfs_mkdir(
			"/d/big/Fichier numéro 01.dat/x", 0), E_NOTDIR);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	h_eq_i64("remontage", fakefat_mount(g_b, "/d"), 0);
}

static void	renommage_fichier(void)
{
	t_vfile		*f;
	t_dirent	d;

	vst_put("/d/x.txt", "x", 1);
	h_eq_i64("renommer", vfs_rename("/d/x.txt", "/d/Nouveau nom.txt"), 0);
	h_eq_i64("casse seule", vfs_rename("/d/nouveau NOM.txt",
			"/d/NOUVEAU NOM.TXT"), 0);
	vfs_open("/d", O_DIRECTORY, &f);
	while (vfs_readdir(f, &d) == 1 && strncmp(d.name, "NOUV", 4) != 0)
		;
	vfs_close(f);
	h_eq_str("nouvelle casse visible", d.name, "NOUVEAU NOM.TXT");
}

static void	renommage_dossier(void)
{
	h_true(vfs_mkdir("/d/m", 0) == 0 && vfs_mkdir("/d/m/sub", 0) == 0
		&& vfs_mkdir("/d/n", 0) == 0, "arbre");
	vst_put("/d/m/sub/f", "f", 1);
	h_eq_i64("déplacer dossier", vfs_rename("/d/m/sub", "/d/n/sub"), 0);
	h_eq_i64("contenu suivi", vst_same("/d/n/sub/f", "f", 1), 0);
	h_eq_i64("dans son sous-arbre", vfs_rename("/d/n", "/d/n/sub/z"),
		E_INVAL);
	h_eq_i64("sur existant", vfs_rename("/d/n/sub/f", "/d/NOUVEAU NOM.TXT"),
		E_EXIST);
	h_eq_i64("source absente", vfs_rename("/d/zz", "/d/yy"), E_NOENT);
	h_eq_i64("fichier vers dossier", vfs_rename("/d/n/sub/f", "/d/m/f2"), 0);
	h_eq_i64("lu après déplacement", vst_same("/d/m/f2", "f", 1), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck (..)", fake_fsck(g_b), 0);
	h_eq_i64("remontage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("démontage final", vfs_umount("/d"), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_ns");
	h_run("dossiers", dossiers);
	h_run("dossier étendu sur plusieurs clusters", dossier_etendu);
	h_run("renommage de fichier", renommage_fichier);
	h_run("renommage et déplacement de dossier", renommage_dossier);
	return (h_end());
}
