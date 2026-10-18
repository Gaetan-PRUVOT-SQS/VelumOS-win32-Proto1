#include "../d10/fake.h"

#define REG_TMP FAKE_REG ".tmp"

static t_pm			g_pm;
static const t_pmfs	*g_reel;
static t_pmfs		g_table;
static int			g_couper;
static char			g_avant[FAKE_DATA];
static uint8_t		g_octets[8];
static size_t		g_noct;

static int	w_write_new(void *ctx, const char *path, t_span data)
{
	if (g_couper && path && (strcmp(path, FAKE_REG) == 0
			|| strcmp(path, REG_TMP) == 0))
	{
		(void)fake_fs_put(path, data.p, data.len / 2);
		return (E_IO);
	}
	return (g_reel->write_new(ctx, path, data));
}

static void	armer(void)
{
	g_reel = g_pm.fs;
	g_table = *g_reel;
	g_table.write_new = w_write_new;
	g_pm.fs = &g_table;
	g_couper = 0;
}

static void	photo(const char *apk)
{
	t_ffile	*f;

	fake_reg_text(g_avant);
	g_noct = 0;
	f = fake_fs_get(apk);
	if (!f || f->len > sizeof(g_octets))
		return ;
	g_noct = f->len;
	memcpy(g_octets, f->data, f->len);
}

static void	registre_intact(const char *quoi)
{
	char	now[FAKE_DATA];

	fake_reg_text(now);
	h_eq_str(quoi, now, g_avant);
	h_true(!fake_fs_get(REG_TMP), "registre provisoire parti");
}

static void	fichier_intact(const char *apk)
{
	t_ffile	*f;

	f = fake_fs_get(apk);
	h_true(f != NULL && f->len == g_noct
		&& memcmp(f->data, g_octets, g_noct) == 0,
		"fichier inchange");
	h_true(!fake_fs_get("/data/apps/com.a.apk.tmp"), "apk provisoire parti");
}

static void	t_neuf(void)
{
	t_pmentry	e;

	fake_setup(&g_pm);
	armer();
	photo("/data/apps/com.n.apk");
	g_couper = 1;
	h_eq_i64("echec propage", fake_install(&g_pm, "com.n", 1, 0xaa), E_IO);
	g_couper = 0;
	registre_intact("registre absent");
	h_true(!fake_fs_get(FAKE_REG), "pas de registre nouveau");
	h_true(!fake_fs_get("/data/apps/com.n.apk"), "apk nouveau absent");
	h_true(!fake_fs_get("/data/apps/com.n.apk.tmp"), "provisoire absent");
	h_eq_i64("introuvable", pm_find(&g_pm, "com.n", &e), E_NOENT);
	h_eq_i64("liste vide", pm_list(&g_pm, &e, 1), 0);
}

static void	t_maj(void)
{
	t_pmentry	e;

	fake_setup(&g_pm);
	armer();
	h_eq_i64("v1", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	photo("/data/apps/com.a.apk");
	g_couper = 1;
	h_eq_i64("echec propage", fake_install(&g_pm, "com.a", 2, 0xaa), E_IO);
	g_couper = 0;
	registre_intact("registre v1");
	fichier_intact("/data/apps/com.a.apk");
	h_eq_i64("toujours v1", pm_find(&g_pm, "com.a", &e), 0);
	h_eq_u64("version registre", e.info.version_code, 1);
	h_eq_u64("version fichier", g_octets[0], 1);
	h_eq_i64("liste", pm_list(&g_pm, &e, 4), 1);
}

static void	t_second(void)
{
	t_pmentry	e;
	t_ffile		*a;

	fake_setup(&g_pm);
	armer();
	h_eq_i64("a", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	photo("/data/apps/com.a.apk");
	g_couper = 1;
	h_eq_i64("b refuse", fake_install(&g_pm, "com.b", 1, 0xbb), E_IO);
	g_couper = 0;
	registre_intact("registre seulement a");
	fichier_intact("/data/apps/com.a.apk");
	h_true(!fake_fs_get("/data/apps/com.b.apk"), "b invisible");
	h_true(!fake_fs_get("/data/apps/com.b.apk.tmp"), "b provisoire parti");
	h_eq_i64("b absent", pm_find(&g_pm, "com.b", &e), E_NOENT);
	h_eq_i64("a connu", pm_find(&g_pm, "com.a", &e), 0);
	h_eq_u64("a version", e.info.version_code, 1);
	h_eq_u64("a signataire", e.info.cert[0], 0xaa);
	a = fake_fs_get("/data/apps/com.a.apk");
	h_true(a && a->data[0] == 1 && a->data[1] == 0xaa, "a non melange");
	h_eq_i64("liste", pm_list(&g_pm, &e, 4), 1);
}

static void	t_temoin(void)
{
	t_pmentry	e;

	fake_setup(&g_pm);
	armer();
	h_eq_i64("neuf", fake_install(&g_pm, "com.n", 1, 0xaa), 0);
	h_eq_i64("visible", pm_find(&g_pm, "com.n", &e), 0);
	h_eq_u64("version", e.info.version_code, 1);
	h_true(fake_fs_get("/data/apps/com.n.apk") != NULL, "fichier pose");
	h_true(!fake_fs_get("/data/apps/com.n.apk.tmp"), "provisoire parti");
	h_true(!fake_fs_get(REG_TMP), "registre provisoire parti");
}

static void	t_deux_points(void)
{
	fake_setup(&g_pm);
	armer();
	h_eq_i64("paquet ..", fake_install(&g_pm, "a..b.c", 1, 0xaa), E_INVAL);
	h_true(!fake_fs_get("/data/apps/a..b.c.apk"), "rien ecrit");
	h_true(!fake_fs_get(FAKE_REG), "registre non cree");
	h_eq_i64("aucune fuite", g_mem.live, 0);
}

int	main(void)
{
	h_begin("a04 ecriture du registre");
	h_run("ecriture_registre_installation_neuve", t_neuf);
	h_run("ecriture_registre_mise_a_jour", t_maj);
	h_run("ecriture_registre_second_paquet", t_second);
	h_run("ecriture_reussie_temoin", t_temoin);
	h_run("paquet_deux_points", t_deux_points);
	return (h_end());
}
