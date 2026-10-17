#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	monter(void)
{
	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	h_eq_i64("formatage", fakefat_format(g_b, 512, 1), 0);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
}

static void	cloturer(void)
{
	g_b->watch = -1;
	g_b->watch_fail = 0;
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck après la panne", fake_fsck(g_b), 0);
	h_eq_u64("aucun accès hors disque", g_b->oob, 0);
	fakeblk_free(g_b);
}

static uint64_t	secteur_du_cluster(uint32_t c)
{
	t_fakegeo	g;

	fakefat_geo(g_b, &g);
	return ((uint64_t)g.rsvd + (uint64_t)g.nfats * g.fatsz
		+ (uint64_t)(c - 2) * g.spc);
}

static const uint8_t	*secteur_image(uint32_t c)
{
	return (g_b->img + secteur_du_cluster(c) * 512);
}

static uint32_t	premier_cluster(const uint8_t *e)
{
	return ((le16(e + 20) << 16) | le16(e + 26));
}

static void	panne_en_ecriture_sur(uint32_t c)
{
	g_b->watch = (int64_t)secteur_du_cluster(c);
	g_b->watch_fail = 1;
}

static void	remplir_racine(const char *prefixe, uint32_t n)
{
	char		chemin[32];
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		snprintf(chemin, sizeof(chemin), "/d/%s%02u.TXT", prefixe, i);
		h_eq_i64("remplissage", vst_put(chemin, "x", 1), 0);
		i++;
	}
}

static void	suppression_partielle_du_nom_long(void)
{
	monter();
	remplir_racine("F", 15);
	h_eq_i64("fichier à nom long", vst_put("/d/Long Name Of File.txt",
			"contenu", 7), 0);
	remplir_racine("G", 14);
	panne_en_ecriture_sur(fake_fat_at(g_b, 2));
	h_eq_i64("renommage de casse avec secteur défaillant",
		vfs_rename("/d/Long Name Of File.txt", "/d/LONG NAME OF FILE.TXT"),
		E_IO);
	g_b->watch = -1;
	g_b->watch_fail = 0;
	cloturer();
}

static void	deplacement_avec_suppression_en_echec(void)
{
	t_vstat	st;

	monter();
	h_eq_i64("dossier A", vfs_mkdir("/d/A", 0), 0);
	h_eq_i64("dossier B", vfs_mkdir("/d/B", 0), 0);
	h_eq_i64("fichier", vst_put("/d/A/F.TXT", "hello", 5), 0);
	panne_en_ecriture_sur(premier_cluster(fake_rootent(g_b, 0)));
	h_eq_i64("déplacement avec secteur source défaillant",
		vfs_rename("/d/A/F.TXT", "/d/B/F.TXT"), E_IO);
	g_b->watch = -1;
	g_b->watch_fail = 0;
	h_eq_i64("destination absente", vfs_stat("/d/B/F.TXT", &st), E_NOENT);
	h_eq_i64("source intacte", vst_same("/d/A/F.TXT", "hello", 5), 0);
	cloturer();
}

static void	deplacement_de_dossier_avec_dotdot_en_echec(void)
{
	t_vstat		st;
	uint32_t	a;
	uint32_t	sub;

	monter();
	h_eq_i64("dossier A", vfs_mkdir("/d/A", 0), 0);
	h_eq_i64("dossier B", vfs_mkdir("/d/B", 0), 0);
	h_eq_i64("sous-dossier", vfs_mkdir("/d/A/SUB", 0), 0);
	a = premier_cluster(fake_rootent(g_b, 0));
	sub = premier_cluster(secteur_image(a) + 2 * 32);
	panne_en_ecriture_sur(sub);
	h_eq_i64("déplacement avec «..» non écrit",
		vfs_rename("/d/A/SUB", "/d/B/SUB"), E_IO);
	g_b->watch = -1;
	g_b->watch_fail = 0;
	h_eq_i64("destination absente", vfs_stat("/d/B/SUB", &st), E_NOENT);
	h_eq_i64("source présente", vfs_stat("/d/A/SUB", &st), 0);
	h_eq_u64("«..» pointe toujours sur A", premier_cluster(
			secteur_image(sub) + 32), a);
	cloturer();
}

int	main(void)
{
	h_begin("a12/revue_rename_panne");
	h_run("renommage de casse : nom long à moitié supprimé",
		suppression_partielle_du_nom_long);
	h_run("déplacement : suppression de la source en échec",
		deplacement_avec_suppression_en_echec);
	h_run("déplacement de dossier : mise à jour de «..» en échec",
		deplacement_de_dossier_avec_dotdot_en_echec);
	return (h_end());
}
