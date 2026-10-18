#include "fake.h"

#define APK_PATH "/data/apps/com.a.apk"
#define REG_TMP "/data/apps/paquets.tmp"
#define APK_TMP "/data/apps/com.a.apk.tmp"
#define APK_BAK "/data/apps/com.a.apk.bak"

typedef struct s_cible
{
	const char	*ecriture;
	const char	*renommage;
	int			apres;
}	t_cible;

static t_cible		g_cible;
static int			g_n_ecriture;
static t_pmfs		g_table;
static const t_pmfs	*g_reel;
static t_pm			g_pm;

static int	w_write_new(void *ctx, const char *path, t_span data)
{
	if (g_cible.ecriture && path && strcmp(path, g_cible.ecriture) == 0)
	{
		g_n_ecriture++;
		if (g_cible.apres <= 0 || g_n_ecriture >= g_cible.apres)
		{
			(void)fake_fs_put(path, data.p, data.len / 2);
			return (E_IO);
		}
	}
	return (g_reel->write_new(ctx, path, data));
}

static int	w_rename(void *ctx, const char *from, const char *to)
{
	if (g_cible.renommage && strcmp(to, g_cible.renommage) == 0)
		return (E_IO);
	return (g_reel->rename(ctx, from, to));
}

static void	prepare(void)
{
	fake_setup(&g_pm);
	g_reel = g_pm.fs;
	g_table = *g_reel;
	g_table.write_new = w_write_new;
	g_table.rename = w_rename;
	g_pm.fs = &g_table;
	memset(&g_cible, 0, sizeof(g_cible));
	g_n_ecriture = 0;
	h_eq_i64("depart v1", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
}

static void	verifie_etat(const char *avant, uint32_t version)
{
	char		now[FAKE_DATA];
	t_pmentry	e;
	t_ffile		*apk;

	fake_reg_text(now);
	apk = fake_fs_get(APK_PATH);
	h_true(apk != NULL, "fichier de l'appli present");
	h_eq_i64("appli connue", pm_find(&g_pm, "com.a", &e), 0);
	h_eq_u64("version du registre", e.info.version_code, version);
	if (apk)
		h_eq_u64("version du fichier", apk->data[0], version);
	if (version == 1)
		h_eq_str("registre inchange", now, avant);
	h_true(!fake_fs_get(REG_TMP), "registre provisoire parti");
	h_true(!fake_fs_get(APK_TMP), "fichier provisoire parti");
	h_true(!fake_fs_get(APK_BAK), "sauvegarde partie");
}

static void	t_echec_ecriture_registre(void)
{
	char	avant[FAKE_DATA];

	prepare();
	fake_reg_text(avant);
	g_cible.ecriture = REG_TMP;
	h_eq_i64("echec propage", fake_install(&g_pm, "com.a", 2, 0xaa), E_IO);
	verifie_etat(avant, 1);
}

static void	t_echec_renommage_registre(void)
{
	char	avant[FAKE_DATA];

	prepare();
	fake_reg_text(avant);
	g_cible.renommage = "/data/apps/paquets";
	h_eq_i64("echec propage", fake_install(&g_pm, "com.a", 2, 0xaa), E_IO);
	verifie_etat(avant, 1);
}

static void	t_echec_renommage_apk(void)
{
	char	avant[FAKE_DATA];

	prepare();
	fake_reg_text(avant);
	g_cible.renommage = APK_PATH;
	h_eq_i64("echec propage", fake_install(&g_pm, "com.a", 2, 0xaa), E_IO);
	verifie_etat(avant, 1);
}

static void	t_echec_ecriture_apk(void)
{
	char	avant[FAKE_DATA];

	prepare();
	fake_reg_text(avant);
	g_cible.ecriture = APK_TMP;
	h_eq_i64("echec propage", fake_install(&g_pm, "com.a", 2, 0xaa), E_IO);
	verifie_etat(avant, 1);
}

static void	t_echec_restauration_registre(void)
{
	char		avant[FAKE_DATA];
	t_pmentry	e;
	t_ffile		*apk;

	prepare();
	fake_reg_text(avant);
	g_cible.ecriture = REG_TMP;
	g_cible.apres = 2;
	g_cible.renommage = APK_PATH;
	h_eq_i64("echec propage", fake_install(&g_pm, "com.a", 2, 0xaa), E_IO);
	verifie_etat(avant, 1);
	apk = fake_fs_get(APK_PATH);
	h_true(apk != NULL && apk->data[0] == 1, "fichier reste v1");
	h_eq_i64("registre relu", pm_find(&g_pm, "com.a", &e), 0);
	h_eq_u64("version presentee", e.info.version_code, 1);
	h_eq_i64("aucune fuite", g_mem.live, 0);
}

static void	t_sauvegarde_sans_memoire(void)
{
	char	avant[FAKE_DATA];

	prepare();
	fake_reg_text(avant);
	fake_mem_reset(2);
	h_eq_i64("copie sans memoire", fake_install(&g_pm, "com.a", 2, 0xaa),
		E_NOMEM);
	fake_mem_reset(0);
	verifie_etat(avant, 1);
	h_eq_i64("aucune fuite", g_mem.live, 0);
}

static void	t_mise_a_jour_reussie(void)
{
	char	avant[FAKE_DATA];

	prepare();
	fake_reg_text(avant);
	h_eq_i64("v2", fake_install(&g_pm, "com.a", 2, 0xaa), 0);
	verifie_etat(avant, 2);
}

static void	verifie_paire(const char *avant, const char *apres, int n)
{
	char		now[FAKE_DATA];
	t_ffile		*apk;
	t_pmentry	e;
	int			ancien;
	int			neuf;

	fake_reg_text(now);
	apk = fake_fs_get(APK_PATH);
	h_true(apk != NULL, "fichier present");
	if (!apk)
		return ;
	ancien = strcmp(now, avant) == 0 && apk->data[0] == 1;
	neuf = strcmp(now, apres) == 0 && apk->data[0] == 2;
	h_true(ancien || neuf, "registre et fichier de la meme version");
	g_fs.fail_at = 0;
	h_eq_i64("appli connue", pm_find(&g_pm, "com.a", &e), 0);
	h_eq_u64("version coherente", e.info.version_code, apk->data[0]);
	(void)n;
}

static void	t_balayage_pannes_uniques(void)
{
	char	avant[FAKE_DATA];
	char	apres[FAKE_DATA];
	int		total;
	int		n;

	prepare();
	fake_reg_text(avant);
	g_fs.ops = 0;
	h_eq_i64("sans panne", fake_install(&g_pm, "com.a", 2, 0xaa), 0);
	total = g_fs.ops;
	fake_reg_text(apres);
	h_true(total >= 4, "plusieurs points de panne");
	n = 1;
	while (n <= total)
	{
		prepare();
		g_fs.ops = 0;
		g_fs.fail_at = n;
		(void)fake_install(&g_pm, "com.a", 2, 0xaa);
		verifie_paire(avant, apres, n);
		n++;
	}
}

int	main(void)
{
	h_begin("e04 mise a jour atomique");
	h_run("maj_echec_ecriture_registre", t_echec_ecriture_registre);
	h_run("maj_echec_renommage_registre", t_echec_renommage_registre);
	h_run("maj_echec_renommage_apk", t_echec_renommage_apk);
	h_run("maj_echec_restauration_registre", t_echec_restauration_registre);
	h_run("maj_sauvegarde_sans_memoire", t_sauvegarde_sans_memoire);
	h_run("maj_echec_ecriture_apk", t_echec_ecriture_apk);
	h_run("maj_reussie", t_mise_a_jour_reussie);
	h_run("maj_balayage_pannes_uniques", t_balayage_pannes_uniques);
	return (h_end());
}
