#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "runcmd.h"

static const t_runrow	g_ok[] = {
{"hello", 0, "/system/bin/hello"},
{"  hello  ", 0, "/system/bin/hello"},
{"\thello\t", 0, "/system/bin/hello"},
{"/system/bin/hello", 0, "/system/bin/hello"},
{"hello.exe", 0, "/system/bin/hello.exe"},
{"a-b_c.d", 0, "/system/bin/a-b_c.d"},
{"A1", 0, "/system/bin/A1"},
{"x", 0, "/system/bin/x"}
};

static const t_runrow	g_ko[] = {
{"", -22, ""}, {"   ", -22, ""}, {"/", -22, ""}, {"/etc/passwd", -22, ""},
{"../hello", -22, ""}, {"bin/hello", -22, ""}, {"hello world", -22, ""},
{"hello;ls", -22, ""}, {"hel`lo", -22, ""}, {".hidden", -22, ""},
{"-x", -22, ""}, {"_x", -22, ""}, {"a..b", -22, ""}, {"..", -22, ""},
{"/system/bin/", -22, ""}, {"/system/bin/../x", -22, ""},
{"/system/binx/hello", -22, ""}, {"/system/bin/a/b", -22, ""},
{"\001hello", -22, ""}, {"h\303\251llo", -22, ""}, {"hello\n", -22, ""},
{"$HOME", -22, ""}, {"a|b", -22, ""}, {"a&b", -22, ""}, {"a>b", -22, ""}
};

static void	run_valides_et_normalisees(void)
{
	char		path[RUN_NAME_MAX + 16];
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_ok) / sizeof(g_ok[0]))
	{
		h_eq_i64(g_ok[i].in,
			run_resolve(g_ok[i].in, strlen(g_ok[i].in), path, sizeof(path)), 0);
		h_eq_str("chemin", path, g_ok[i].path);
		i++;
	}
}

static void	run_refusees(void)
{
	char		path[RUN_NAME_MAX + 16];
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_ko) / sizeof(g_ko[0]))
	{
		h_eq_i64(g_ko[i].in,
			run_resolve(g_ko[i].in, strlen(g_ko[i].in), path, sizeof(path)),
			-22);
		i++;
	}
	h_eq_i64("NUL integre", run_resolve("he\0llo", 6, path, sizeof(path)), -22);
}

static void	run_limites_de_taille(void)
{
	char	in[RUN_INPUT_MAX + 8];
	char	path[RUN_NAME_MAX + 16];

	memset(in, 'a', sizeof(in));
	h_eq_i64("63 caracteres", run_resolve(in, 63, path, sizeof(path)), 0);
	h_eq_u64("63: longueur du chemin", strlen(path), 12 + 63);
	h_eq_i64("64 caracteres", run_resolve(in, 64, path, sizeof(path)), -22);
	h_eq_i64("entree geante", run_resolve(in, RUN_INPUT_MAX + 1, path,
			sizeof(path)), -22);
	h_eq_i64("tampon exact", run_resolve("hello", 5, path, 18), 0);
	h_eq_i64("tampon trop court d'un octet", run_resolve("hello", 5, path,
			17), -34);
	h_eq_i64("tampon nul", run_resolve("hello", 5, path, 0), -22);
	h_eq_i64("entree nulle", run_resolve(NULL, 5, path, sizeof(path)), -22);
	h_eq_i64("sortie nulle", run_resolve("hello", 5, NULL, 20), -22);
}

int	main(void)
{
	h_begin("a20/runcmd");
	h_run("executer: valides et normalisees", run_valides_et_normalisees);
	h_run("executer: refusees", run_refusees);
	h_run("executer: limites de taille", run_limites_de_taille);
	return (h_end());
}
