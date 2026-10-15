#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	ajout_secteur_en_panne(void)
{
	t_vfile	*f;
	t_vstat	st;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	g_b->watch = 1;
	g_b->watch_fail = 1;
	h_eq_i64("créer", vfs_open("/d/a.txt", O_CREAT | O_WRONLY | O_APPEND, &f),
		0);
	h_eq_i64("écriture faite malgré FSInfo", vfs_write(f, "12345", 5), 5);
	h_true(g_b->watch_writes > 0, "écriture de FSInfo tentée");
	h_true(vfs_fstat(f, &st) == 0 && st.size == 5, "taille après écriture");
	h_eq_i64("seconde écriture", vfs_write(f, "678", 3), 3);
	h_true(vfs_fstat(f, &st) == 0 && st.size == 8, "rien de dupliqué");
	h_eq_i64("fsync rend l'erreur", vfs_fsync(f), E_IO);
	h_eq_i64("fsync la rend encore", vfs_fsync(f), E_IO);
	h_true(fake_free_gap(g_b) != 0, "FSInfo en retard pendant la panne");
	h_eq_i64("fermer", vfs_close(f), 0);
	h_eq_i64("contenu", vst_same("/d/a.txt", "12345678", 8), 0);
}

static void	noms_secteur_en_panne(void)
{
	t_vfile	*f;
	t_vstat	st;

	h_eq_i64("mkdir fait", vfs_mkdir("/d/sous", 0), 0);
	h_eq_i64("création O_EXCL faite", vfs_open("/d/sous/neuf.txt",
			O_CREAT | O_EXCL | O_RDWR, &f), 0);
	h_eq_i64("troncature faite", vfs_truncate(f, 0), 0);
	h_eq_i64("fermer", vfs_close(f), 0);
	h_eq_i64("renommage fait", vfs_rename("/d/sous/neuf.txt",
			"/d/sous/Renommé.txt"), 0);
	h_eq_i64("ancien nom absent", vfs_stat("/d/sous/neuf.txt", &st), E_NOENT);
	h_eq_i64("nouveau nom présent", vfs_stat("/d/sous/Renommé.txt", &st), 0);
	h_eq_i64("suppression faite", vfs_unlink("/d/sous/Renommé.txt"), 0);
	h_eq_i64("fichier absent", vfs_stat("/d/sous/Renommé.txt", &st), E_NOENT);
	h_true(fake_warns() >= 1 && fake_warns() <= 4, "avertissement borné");
}

static void	secteur_revenu(void)
{
	t_vfile	*f;

	g_b->watch_fail = 0;
	h_eq_i64("suppression après panne", vfs_unlink("/d/a.txt"), 0);
	h_eq_i64("FSInfo juste dès l'opération suivante", fake_free_gap(g_b), 0);
	g_b->watch_fail = 1;
	h_eq_i64("écrire", vst_put("/d/b.txt", "b", 1), E_IO);
	h_eq_i64("fichier écrit quand même", vst_same("/d/b.txt", "b", 1), 0);
	g_b->watch_fail = 0;
	h_eq_i64("ouvrir", vfs_open("/d/b.txt", O_RDONLY, &f), 0);
	h_eq_i64("fsync réussit", vfs_fsync(f), 0);
	h_eq_i64("FSInfo juste après fsync", fake_free_gap(g_b), 0);
	h_eq_i64("fermer", vfs_close(f), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

static void	compteur_inconnu_sans_ecriture(void)
{
	t_vfile		*f;
	uint64_t	avant;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	fake_fsi_set(g_b, FAKE_FSI_FREE, FAT_UNKNOWN);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("écrire a", vst_put("/d/a.bin", "0123456789", 10), 0);
	h_eq_i64("écrire b", vst_put("/d/b.bin", "0123456789", 10), 0);
	g_b->watch = 1;
	avant = g_b->watch_writes;
	h_eq_i64("supprimer", vfs_unlink("/d/a.bin"), 0);
	h_eq_u64("suppression : FSInfo non réécrit", g_b->watch_writes, avant);
	h_eq_i64("ouvrir", vfs_open("/d/b.bin", O_RDWR, &f), 0);
	h_eq_i64("tronquer", vfs_truncate(f, 0), 0);
	h_eq_u64("troncature : FSInfo non réécrit", g_b->watch_writes, avant);
	h_eq_i64("fermer", vfs_close(f), 0);
	h_eq_u64("compteur resté inconnu", fake_fsi(g_b, FAKE_FSI_FREE),
		FAT_UNKNOWN);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12 fat fsinfo pannes");
	h_run("B1 : ajout, secteur FSInfo en panne", ajout_secteur_en_panne);
	h_run("B1 : création, renommage, suppression", noms_secteur_en_panne);
	h_run("B1 : secteur redevenu écrivable", secteur_revenu);
	h_run("B2 : compteur inconnu, aucune écriture",
		compteur_inconnu_sans_ecriture);
	return (h_end());
}
