#include <stdint.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "runcmd.h"

static const char	*g_bad_apk[] = {
	"",
	"a.apk",
	"/.apk",
	"/data/a.txt",
	"/data/a.apk.txt",
	"/data/../a.apk",
	"/data//a.apk",
	"/data/a b.apk",
	"/data/\xc3\xa9.apk",
	"/data/a.APK",
	"/data/a\x7f.apk",
	"hello"
};

static void	apk_valides(void)
{
	char	path[RUN_APK_MAX];

	h_eq_i64("chemin systeme", run_apk_path("/system/apps/a.apk", 18, path,
			sizeof(path)), 0);
	h_eq_str("copie exacte", path, "/system/apps/a.apk");
	h_eq_i64("espaces autour", run_apk_path("  /data/apps/b.c.apk\t", 21, path,
			sizeof(path)), 0);
	h_eq_str("rogne", path, "/data/apps/b.c.apk");
	h_eq_i64("plus court chemin", run_apk_path("/a.apk", 6, path,
			sizeof(path)), 0);
}

static void	apk_refuses(void)
{
	char	path[RUN_APK_MAX];
	size_t	i;

	i = 0;
	while (i < sizeof(g_bad_apk) / sizeof(g_bad_apk[0]))
	{
		h_eq_i64(g_bad_apk[i], run_apk_path(g_bad_apk[i], strlen(g_bad_apk[i]),
				path, sizeof(path)), E_INVAL);
		i++;
	}
	h_eq_i64("entree nulle", run_apk_path(NULL, 4, path, sizeof(path)),
		E_INVAL);
	h_eq_i64("sortie nulle", run_apk_path("/a.apk", 6, NULL, 0), E_INVAL);
}

static void	apk_limites_de_taille(void)
{
	char	in[RUN_INPUT_MAX + 8];
	char	path[RUN_APK_MAX];

	memset(in, 'a', sizeof(in));
	in[0] = '/';
	memcpy(in + RUN_APK_MAX - 5, ".apk", 4);
	h_eq_i64("127 octets", run_apk_path(in, RUN_APK_MAX - 1, path,
			sizeof(path)), 0);
	h_eq_u64("termine", strlen(path), RUN_APK_MAX - 1);
	memset(in, 'a', sizeof(in));
	in[0] = '/';
	memcpy(in + RUN_APK_MAX - 4, ".apk", 4);
	h_eq_i64("128 octets", run_apk_path(in, RUN_APK_MAX, path, sizeof(path)),
		E_INVAL);
	h_eq_i64("tampon trop petit", run_apk_path("/data/a.apk", 11, path, 11),
		E_INVAL);
	h_eq_i64("entree trop longue", run_apk_path(in, RUN_INPUT_MAX + 1, path,
			sizeof(path)), E_INVAL);
}

int	main(void)
{
	h_begin("a20/executer-apk");
	h_run("executer: apk valides", apk_valides);
	h_run("executer: apk refuses", apk_refuses);
	h_run("executer: apk, limites de taille", apk_limites_de_taille);
	return (h_end());
}
