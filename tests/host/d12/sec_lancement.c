#include <stdlib.h>
#include <stdio.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "apkrun.h"
#include "apkglue.h"
#include "../d10/fake.h"

static t_apkrun	g_app;
static char		g_pkg[PM_PKG_MAX];
static uint8_t	g_cert[PM_CERT_LEN];
static uint8_t	g_file[4] = {1, 2, 3, 4};
static uint8_t	g_dexb[4] = {0x64, 0x65, 0x78, 0x0a};
static char		g_vm_obj;
static int		g_reads;
static int		g_verify;
static int		g_dex_loads;

static void	raz(void)
{
	fake_fs_reset();
	fake_mem_reset(0);
	g_mem.live = 0;
	memset(g_pkg, 0, sizeof(g_pkg));
	strlcpy(g_pkg, "com.a", sizeof(g_pkg));
	memset(g_cert, 0xaa, sizeof(g_cert));
}

static void	poser(const char *root, uint8_t signer)
{
	char	hex[PM_CERT_LEN * 2 + 1];
	char	line[512];
	char	path[PM_PATH_MAX];
	size_t	i;
	int		n;

	i = 0;
	while (i < PM_CERT_LEN)
	{
		hex[2 * i] = "0123456789abcdef"[signer >> 4];
		hex[2 * i + 1] = "0123456789abcdef"[signer & 15];
		i++;
	}
	hex[sizeof(hex) - 1] = '\0';
	n = snprintf(path, sizeof(path), "%s/paquets", root);
	h_true(n > 0 && (size_t)n < sizeof(path), "chemin du registre");
	n = snprintf(line, sizeof(line), "%s;1;Appli;Lx/y/Main;;%s\n", g_pkg, hex);
	h_true(n > 0 && (size_t)n < sizeof(line), "ligne d'ancre");
	h_eq_i64("ancre posee", fake_fs_put(path, line, (size_t)n), 0);
}

static int	lance(const char *path)
{
	int	r;

	memset(&g_app, 0, sizeof(g_app));
	g_reads = 0;
	g_verify = 0;
	g_dex_loads = 0;
	r = apkrun_load(&g_app, path);
	free(g_app.d);
	g_app.d = NULL;
	return (r);
}

static void	refuse_signataire(const char *nom)
{
	h_eq_i64(nom, lance("/data/apps/com.a.apk"), -1);
	h_eq_i64("signature pourtant valide", g_verify, 1);
	h_eq_i64("code non charge", g_dex_loads, 0);
	h_eq_str("motif", g_app.msg, T_SIGNER);
	h_eq_i64("aucune fuite", g_mem.live, 0);
}

static void	t_ancre_absente(void)
{
	raz();
	refuse_signataire("sans registre");
}

static void	t_ancre_differente(void)
{
	raz();
	poser(APKRUN_DATA_ROOT, 0xaa);
	memset(g_cert, 0xbb, sizeof(g_cert));
	refuse_signataire("certificat different");
	raz();
	poser(APKRUN_DATA_ROOT, 0xaa);
	g_cert[PM_CERT_LEN - 1] = 0xab;
	refuse_signataire("dernier octet");
	raz();
	poser(APKRUN_DATA_ROOT, 0x00);
	memset(g_cert, 0, sizeof(g_cert));
	g_cert[0] = 0x01;
	refuse_signataire("premier octet sur ancre nulle");
}

static void	t_ancre_conforme(void)
{
	raz();
	poser(APKRUN_DATA_ROOT, 0xaa);
	h_eq_i64("ancre egale", lance("/data/apps/com.a.apk"), 0);
	h_eq_i64("code charge", g_dex_loads, 1);
	h_eq_i64("aucune fuite", g_mem.live, 0);
	raz();
	memset(g_cert, 0, sizeof(g_cert));
	poser(APKRUN_DATA_ROOT, 0x00);
	h_eq_i64("ancre nulle conforme", lance("/data/apps/com.a.apk"), 0);
	h_eq_i64("code charge", g_dex_loads, 1);
	raz();
	poser(APKRUN_SYS_ROOT, 0xaa);
	h_eq_i64("ancre systeme", lance("/system/apps/com.a.apk"), 0);
	h_eq_i64("code charge", g_dex_loads, 1);
}

static void	t_registre_ferme(void)
{
	raz();
	poser(APKRUN_DATA_ROOT, 0xaa);
	g_fs.ops = 0;
	g_fs.fail_at = 1;
	h_eq_i64("lecture en panne", lance("/data/apps/com.a.apk"), -1);
	h_eq_i64("code non charge", g_dex_loads, 0);
	h_eq_str("motif", g_app.msg, T_REGISTRY);
	raz();
	poser(APKRUN_DATA_ROOT, 0xaa);
	fake_mem_reset(1);
	h_eq_i64("memoire en panne", lance("/data/apps/com.a.apk"), -1);
	h_eq_str("motif memoire", g_app.msg, T_REGISTRY);
	h_eq_i64("aucune fuite", g_mem.live, 0);
	h_eq_i64("code non charge", g_dex_loads, 0);
}

static void	t_chemin(void)
{
	char	longu[APKGLUE_PATH_MAX + 8];
	size_t	i;

	raz();
	poser(APKRUN_DATA_ROOT, 0xaa);
	h_eq_i64("deux points", lance("../etc/passwd"), -1);
	h_eq_i64("pas de lecture", g_reads, 0);
	h_eq_i64("pas de signature", g_verify, 0);
	h_eq_str("motif fichier", g_app.msg, T_FILE);
	h_eq_i64("nom piege", lance("/data/apps/foo..bar.apk"), -1);
	h_eq_i64("pas de lecture", g_reads, 0);
	h_eq_i64("chemin vide", lance(""), -1);
	h_eq_i64("pas de lecture", g_reads, 0);
	h_eq_i64("chemin nul", lance(NULL), -1);
	h_eq_i64("pas de lecture", g_reads, 0);
	i = 0;
	while (i < sizeof(longu) - 1)
	{
		longu[i] = 'a';
		i++;
	}
	longu[i] = '\0';
	h_eq_i64("chemin trop long", lance(longu), -1);
	h_eq_i64("pas de lecture", g_reads, 0);
	h_eq_i64("point simple", lance("/data/apps/com.a.apk"), 0);
	h_eq_i64("lecture faite", g_reads, 1);
	h_eq_i64("appli nulle", apkrun_load(NULL, "/data/apps/com.a.apk"), -1);
}

int	apk_open(t_apk *a, t_span file)
{
	(void)file;
	if (a)
		memset(a, 0, sizeof(*a));
	return (0);
}

int	apk_verify(const t_apk *a, t_apksig *out)
{
	(void)a;
	g_verify++;
	memset(out, 0, sizeof(*out));
	memcpy(out->cert_sha256, g_cert, PM_CERT_LEN);
	return (0);
}

int	apk_manifest(const t_apk *a, t_apkmanifest *out)
{
	(void)a;
	memset(out, 0, sizeof(*out));
	memcpy(out->package, g_pkg, PM_PKG_MAX);
	return (0);
}

int64_t	apk_read(const t_apk *a, const char *name, uint8_t **out)
{
	(void)a;
	(void)name;
	*out = g_dexb;
	return ((int64_t)sizeof(g_dexb));
}

const char	*apk_reason(int reason)
{
	(void)reason;
	return (T_APK);
}

int64_t	apkglue_read_file(const char *path, uint8_t **out)
{
	(void)path;
	g_reads++;
	*out = g_file;
	return ((int64_t)sizeof(g_file));
}

static int	stub_mkdir(void *ctx, const char *path)
{
	(void)ctx;
	(void)path;
	return (E_EXIST);
}

const t_pmfs	*apkglue_fs(void)
{
	static t_pmfs	table;
	static int		pret;

	if (!pret)
	{
		table = *fake_fs_table();
		table.mkdir = stub_mkdir;
		pret = 1;
	}
	return (&table);
}

int	apkglue_inspect(t_span apk, t_pminfo *out)
{
	(void)apk;
	(void)out;
	return (E_INVAL);
}

int	apkrun_refuse(t_apkrun *app, const char *reason)
{
	size_t	n;

	n = 0;
	if (reason)
	{
		while (reason[n] && n + 1 < sizeof(app->msg))
		{
			app->msg[n] = reason[n];
			n++;
		}
	}
	app->msg[n] = '\0';
	app->dead = 1;
	return (-1);
}

void	apkrun_log_step(t_apkrun *app, const char *what, const char *more)
{
	(void)app;
	(void)what;
	(void)more;
}

void	apkrun_applog(void *user, uint32_t level, const char *tag, const char *msg)
{
	(void)user;
	(void)level;
	(void)tag;
	(void)msg;
}

uint64_t	apkrun_clock(void *user)
{
	(void)user;
	return (0);
}

int	dvm_create(t_dvm **out, const t_dlimits *lim)
{
	(void)lim;
	*out = (t_dvm *)&g_vm_obj;
	return (0);
}

int	droid_install(t_droid *d, t_dvm *vm)
{
	(void)d;
	(void)vm;
	return (0);
}

int	dvm_load_dex(t_dvm *vm, t_span dex)
{
	(void)vm;
	(void)dex;
	g_dex_loads++;
	return (0);
}

int	main(void)
{
	h_begin("a04 signataire au lancement");
	h_run("ancre_absente", t_ancre_absente);
	h_run("ancre_differente", t_ancre_differente);
	h_run("ancre_conforme", t_ancre_conforme);
	h_run("registre_illisible", t_registre_ferme);
	h_run("chemin_borne", t_chemin);
	return (h_end());
}
