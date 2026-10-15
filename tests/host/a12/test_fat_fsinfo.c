#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"
#include "velum/libk.h"

static t_fakeblk	*g_b;

static void	fichier_ecrit_puis_tronque(void)
{
	t_vfile	*f;
	uint8_t	buf[3000];

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("écart au montage", fake_free_gap(g_b), 0);
	memset(buf, 7, sizeof(buf));
	h_eq_i64("créer", vfs_open("/d/a.bin", O_CREAT | O_RDWR, &f), 0);
	h_eq_i64("écart après création", fake_free_gap(g_b), 0);
	h_eq_i64("écrire 3000", vfs_pwrite(f, buf, 3000, 0), 3000);
	h_eq_i64("écart après écriture", fake_free_gap(g_b), 0);
	h_eq_i64("tronquer à 600", vfs_truncate(f, 600), 0);
	h_eq_i64("écart après troncature partielle", fake_free_gap(g_b), 0);
	h_eq_i64("agrandir à 2000", vfs_truncate(f, 2000), 0);
	h_eq_i64("écart après agrandissement", fake_free_gap(g_b), 0);
	h_eq_i64("tronquer à 0", vfs_truncate(f, 0), 0);
	h_eq_i64("écart après troncature totale", fake_free_gap(g_b), 0);
	h_eq_i64("fermer", vfs_close(f), 0);
	h_eq_i64("supprimer vide", vfs_unlink("/d/a.bin"), 0);
	h_eq_i64("écart après suppression vide", fake_free_gap(g_b), 0);
}

static void	renommage_puis_suppression(void)
{
	uint32_t	avant;

	avant = fake_free_real(g_b);
	h_eq_i64("un cluster", vst_put("/d/tmp.bin", "0123456789", 10), 0);
	h_eq_u64("un cluster pris", fake_free_real(g_b), avant - 1);
	h_eq_i64("écart après chaîne d'un cluster", fake_free_gap(g_b), 0);
	h_eq_i64("renommer", vfs_rename("/d/tmp.bin", "/d/Renommé.bin"), 0);
	h_eq_i64("écart après renommage", fake_free_gap(g_b), 0);
	h_eq_i64("supprimer", vfs_unlink("/d/Renommé.bin"), 0);
	h_eq_u64("cluster rendu", fake_free_real(g_b), avant);
	h_eq_i64("écart après suppression P24", fake_free_gap(g_b), 0);
}

static void	dossier_etendu(uint32_t n, int creer)
{
	char	path[64];
	t_vfile	*f;
	int		rc;

	while (n-- > 0)
	{
		strlcpy(path, "/d/sous/Un nom assez long numéro A.txt", sizeof(path));
		path[strlen(path) - 5] = (char)('A' + n);
		rc = vfs_unlink(path);
		if (creer)
			rc = vfs_open(path, O_CREAT | O_RDWR, &f);
		if (creer && rc == 0)
			rc = vfs_close(f);
		h_eq_i64(path, rc, 0);
	}
}

static void	dossier_cree_puis_supprime(void)
{
	uint32_t	avant;

	avant = fake_free_real(g_b);
	h_eq_i64("mkdir", vfs_mkdir("/d/sous", 0), 0);
	h_eq_u64("cluster du dossier", fake_free_real(g_b), avant - 1);
	h_eq_i64("écart après mkdir", fake_free_gap(g_b), 0);
	dossier_etendu(12, 1);
	h_true(fake_free_real(g_b) < avant - 1, "dossier étendu");
	h_eq_i64("écart après extension du dossier", fake_free_gap(g_b), 0);
	dossier_etendu(12, 0);
	h_eq_i64("renommer dossier", vfs_rename("/d/sous", "/d/Autre nom"), 0);
	h_eq_i64("écart après renommage du dossier", fake_free_gap(g_b), 0);
	h_eq_i64("supprimer dossier", vfs_unlink("/d/Autre nom"), 0);
	h_eq_u64("clusters du dossier rendus", fake_free_real(g_b), avant);
	h_eq_i64("écart après suppression du dossier", fake_free_gap(g_b), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12 fat fsinfo");
	h_run("compteur : fichier écrit, tronqué", fichier_ecrit_puis_tronque);
	h_run("compteur : renommage, suppression", renommage_puis_suppression);
	h_run("compteur : dossier créé, supprimé", dossier_cree_puis_supprime);
	return (h_end());
}
