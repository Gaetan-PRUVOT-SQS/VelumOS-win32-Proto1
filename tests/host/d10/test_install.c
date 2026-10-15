#include <stdio.h>
#include "fake.h"

static t_pm	g_pm;

static void	t_update(void)
{
	t_pmentry	e;

	fake_setup(&g_pm);
	h_eq_i64("neuf", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	h_true(fake_fs_get("/data/apps/com.a.apk") != NULL, "fichier pose");
	h_true(!fake_fs_get("/data/apps/com.a.apk.tmp"), "provisoire parti");
	h_true(!fake_fs_get("/data/apps/paquets.tmp"), "registre provisoire parti");
	h_eq_i64("mise a jour", fake_install(&g_pm, "com.a", 2, 0xaa), 0);
	h_eq_i64("version egale", fake_install(&g_pm, "com.a", 2, 0xaa), 0);
	h_eq_i64("version plus petite", fake_install(&g_pm, "com.a", 1, 0xaa),
		E_INVAL);
	h_eq_i64("autre signataire", fake_install(&g_pm, "com.a", 3, 0xbb),
		E_ACCES);
	h_eq_i64("autre casse", fake_install(&g_pm, "Com.A", 3, 0xaa), E_EXIST);
	h_eq_i64("trouve", pm_find(&g_pm, "com.a", &e), 0);
	h_eq_u64("version gardee", e.info.version_code, 2);
	h_eq_u64("origine", e.origin, PM_ORIGIN_DATA);
	h_eq_str("chemin", e.path, "/data/apps/com.a.apk");
	h_eq_u64("fichier du bon signataire",
		fake_fs_get("/data/apps/com.a.apk")->data[1], 0xaa);
	h_eq_u64("fichier de la version 2",
		fake_fs_get("/data/apps/com.a.apk")->data[0], 2);
}

static void	t_refus(void)
{
	char	sys[256];

	fake_setup(&g_pm);
	snprintf(sys, sizeof(sys), "sys.app;1;S;Ls/A;;%s\n", fake_hex());
	fake_fs_put(FAKE_SYSREG, sys, strlen(sys));
	h_eq_i64("preinstalle", fake_install(&g_pm, "sys.app", 9, 0x01), E_PERM);
	h_eq_i64("preinstalle casse", fake_install(&g_pm, "SYS.app", 9, 1), E_PERM);
	g_insp.err = E_NOTSUP;
	h_eq_i64("examen en erreur", fake_install(&g_pm, "com.a", 1, 1), E_NOTSUP);
	g_insp.err = 0;
	h_eq_i64("paquet hostile", fake_install(&g_pm, "../x.y", 1, 1), E_INVAL);
	g_pm.max_apk = 4;
	h_eq_i64("apk trop gros", fake_install(&g_pm, "com.a", 1, 1), E_RANGE);
	g_pm.max_apk = PM_APK_MAX;
	g_pm.max_packages = 3;
	h_eq_i64("deuxieme", fake_install(&g_pm, "com.a", 1, 1), 0);
	h_eq_i64("troisieme", fake_install(&g_pm, "com.b", 1, 1), 0);
	h_eq_i64("registre plein", fake_install(&g_pm, "com.c", 1, 1), E_NOSPC);
	h_eq_i64("mise a jour a plein", fake_install(&g_pm, "com.b", 2, 1), 0);
	h_true(!fake_fs_get("/data/apps/com.c.apk"), "rien ecrit au refus");
	h_eq_i64("liste coherente", fake_listed_ok(&g_pm), 3);
	g_pm.max_packages = 65;
	h_eq_i64("plafond invalide", fake_install(&g_pm, "com.d", 1, 1), E_INVAL);
}

static void	t_query(void)
{
	static t_pmentry	e[8];
	char				reg[512];

	fake_setup(&g_pm);
	snprintf(reg, sizeof(reg), "sys.app;1;S;Ls/A;;%s\n", fake_hex());
	fake_fs_put(FAKE_SYSREG, reg, strlen(reg));
	snprintf(reg, sizeof(reg), "sys.app;9;Faux;Lf/F;;%s\ncom.b;1;B;Lb/B;;%s\n",
		fake_hex(), fake_hex());
	fake_fs_put(FAKE_REG, reg, strlen(reg));
	fake_fs_put("/data/apps/com.b.apk", "x", 1);
	h_eq_i64("liste sans le faux", pm_list(&g_pm, e, 8), 2);
	h_eq_u64("preinstalle d'abord", e[0].origin, PM_ORIGIN_SYSTEM);
	h_eq_u64("version du preinstalle", e[0].info.version_code, 1);
	h_eq_i64("liste plafonnee", pm_list(&g_pm, e, 1), 1);
	h_eq_i64("trouve preinstalle", pm_find(&g_pm, "sys.app", e), 0);
	h_eq_str("chemin systeme", e[0].path, "/system/apps/sys.app.apk");
	h_eq_i64("inconnu", pm_find(&g_pm, "no.pe", e), E_NOENT);
	h_eq_i64("retrait preinstalle", pm_remove(&g_pm, "sys.app"), E_PERM);
	h_eq_i64("retrait inconnu", pm_remove(&g_pm, "no.pe"), E_NOENT);
	h_eq_i64("retrait", pm_remove(&g_pm, "com.b"), 0);
	h_true(!fake_fs_get("/data/apps/com.b.apk"), "fichier retire");
	h_eq_i64("plus liste", pm_find(&g_pm, "com.b", e), E_NOENT);
	g_pm.max_registry = 16;
	h_eq_i64("registre trop gros", pm_list(&g_pm, e, 8), E_OVERFLOW);
}

static void	t_mem(void)
{
	t_pmentry	e;

	fake_setup(&g_pm);
	h_eq_i64("base", fake_install(&g_pm, "com.a", 1, 1), 0);
	fake_mem_reset(1);
	h_eq_i64("installation sans memoire", fake_install(&g_pm, "com.b", 1, 1),
		E_NOMEM);
	fake_mem_reset(1);
	h_eq_i64("retrait sans memoire", pm_remove(&g_pm, "com.a"), E_NOMEM);
	fake_mem_reset(1);
	h_eq_i64("liste sans memoire", pm_list(&g_pm, &e, 1), E_NOMEM);
	fake_mem_reset(1);
	h_eq_i64("recherche sans memoire", pm_find(&g_pm, "com.a", &e), E_NOMEM);
	fake_mem_reset(0);
	h_eq_i64("etat intact", fake_listed_ok(&g_pm), 1);
	h_eq_i64("une allocation par operation", g_mem.calls, 1);
	h_eq_i64("aucune fuite", g_mem.live, 0);
}

int	main(void)
{
	h_begin("d10 installation");
	h_run("installation_table_neuf_maj_signataire_version", t_update);
	h_run("installation_table_preinstalle_examen_plein", t_refus);
	h_run("liste_recherche_retrait_deux_racines", t_query);
	h_run("allocation_en_echec_une_a_une", t_mem);
	return (h_end());
}
