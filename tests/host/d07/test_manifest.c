#include <stdlib.h>
#include "harness.h"
#include "d07.h"

static void	manifest_bonjour(void)
{
	t_span			f;
	t_apk			a;
	t_apkmanifest	m;

	f = d07_load("ok.apk");
	h_eq_i64("ouverture", apk_open(&a, f), 0);
	h_eq_i64("manifeste", apk_manifest(&a, &m), 0);
	h_eq_str("paquet", m.package, "com.velum.bonjour");
	h_eq_u64("versionCode", m.version_code, 1);
	h_eq_str("versionName", m.version_name, "1.0");
	h_eq_u64("minSdk", m.min_sdk, 21);
	h_eq_u64("targetSdk", m.target_sdk, 34);
	h_eq_str("libelle par reference", m.label, "Bonjour APK");
	h_eq_str("activite", m.activity, "com.velum.bonjour.Principale");
	h_eq_str("descripteur", m.activity_desc, "Lcom/velum/bonjour/Principale;");
	h_eq_u64("aucune permission", m.perm_count, 0);
	h_eq_i64("raison", m.reason, APKR_OK);
	d07_free(f);
}

static void	manifest_nom_relatif(void)
{
	t_span			f;
	t_apk			a;
	t_apkmanifest	m;

	f = d07_load("relatif.apk");
	h_eq_i64("ouverture", apk_open(&a, f), 0);
	h_eq_i64("manifeste", apk_manifest(&a, &m), 0);
	h_eq_str("activite completee", m.activity, "com.velum.relatif.Principale");
	h_eq_str("descripteur", m.activity_desc, "Lcom/velum/relatif/Principale;");
	h_eq_str("libelle en clair", m.label, "Essai relatif");
	h_eq_u64("versionCode", m.version_code, 7);
	h_eq_u64("deux permissions", m.perm_count, 2);
	h_eq_str("permission 0", m.perms[0], "android.permission.INTERNET");
	h_eq_str("permission 1", m.perms[1], "android.permission.VIBRATE");
	d07_free(f);
}

static void	read_entree_et_echecs(void)
{
	t_span	f;
	t_apk	a;
	uint8_t	*buf;

	f = d07_load("ok.apk");
	h_eq_i64("ouverture", apk_open(&a, f), 0);
	h_true(apk_read(&a, "classes.dex", &buf) > 8, "classes.dex extrait");
	h_true(buf != NULL && buf[0] == 'd' && buf[1] == 'e' && buf[2] == 'x',
		"magie dex");
	free(buf);
	h_eq_i64("entree absente", apk_read(&a, "absent", &buf), E_NOENT);
	h_true(buf == NULL, "tampon nul si absente");
	h_eq_i64("sortie nulle", apk_read(&a, "classes.dex", NULL), E_INVAL);
	d07_fail_after(0);
	h_eq_i64("malloc en echec", apk_read(&a, "classes.dex", &buf), E_NOMEM);
	d07_fail_after(-1);
	h_true(buf == NULL, "tampon nul si malloc echoue");
	d07_free(f);
}

static void	manifest_malloc_en_echec(void)
{
	t_span			f;
	t_apk			a;
	t_apkmanifest	m;
	int				r;

	f = d07_load("ok.apk");
	h_eq_i64("ouverture", apk_open(&a, f), 0);
	d07_fail_after(0);
	r = apk_manifest(&a, &m);
	d07_fail_after(-1);
	h_eq_i64("premiere allocation", r, E_NOMEM);
	h_eq_i64("raison memoire", m.reason, APKR_MEMOIRE);
	d07_fail_after(1);
	r = apk_manifest(&a, &m);
	d07_fail_after(-1);
	h_eq_i64("allocation des ressources", r, E_NOMEM);
	h_eq_i64("raison memoire", m.reason, APKR_MEMOIRE);
	h_eq_str("sortie videe au refus", m.package, "");
	d07_free(f);
}

int	main(void)
{
	h_begin("d07/manifest");
	h_run("manifest_bonjour", manifest_bonjour);
	h_run("manifest_nom_relatif", manifest_nom_relatif);
	h_run("read_entree_et_echecs", read_entree_et_echecs);
	h_run("manifest_malloc_en_echec", manifest_malloc_en_echec);
	return (h_end());
}
