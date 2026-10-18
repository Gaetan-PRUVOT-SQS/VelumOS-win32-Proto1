#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "e15.h"

static void	decoupage(void)
{
	char	l1[] = "  echo   a\tb ";
	char	l2[] = "echo \"x  y\" 'z w'\"!\"";
	char	l3[] = "";
	char	*av[CON_ARGS_MAX];

	h_eq_i64("trois mots", con_split(l1, av, CON_ARGS_MAX), 3);
	h_eq_str("mot 2", av[1], "a");
	h_eq_str("mot 3", av[2], "b");
	h_eq_i64("guillemets", con_split(l2, av, CON_ARGS_MAX), 3);
	h_eq_str("double guillemet", av[1], "x  y");
	h_eq_str("simple puis double accoles", av[2], "z w!");
	h_eq_i64("ligne vide", con_split(l3, av, CON_ARGS_MAX), 0);
}

static void	decoupage_invalide(void)
{
	char	l1[] = "echo \"ouvert";
	char	l2[] = "a b c d";
	char	l3[] = "a b c d";
	char	*av[CON_ARGS_MAX];

	h_eq_i64("guillemet non ferme", con_split(l1, av, CON_ARGS_MAX), E_INVAL);
	h_eq_i64("trop d'arguments", con_split(l2, av, 3), E_RANGE);
	h_eq_i64("pile a la limite", con_split(l3, av, 4), 4);
}

typedef struct s_pcase
{
	const char	*cwd;
	const char	*arg;
	const char	*want;
}	t_pcase;

static void	chemins(void)
{
	static const t_pcase	t[] = {
	{"/", ".", "/"},
	{"/", "..", "/"},
	{"/docs", "../docs/./x", "/docs/x"},
	{"/docs", "//a///b/", "/a/b"},
	{"/a/b", "../../..", "/"},
	{"/system/bin", "../etc", "/system/etc"},
	{"/docs", "../../..", "/"},
	{"/docs", "/docs//./Zoo/", "/docs/Zoo"},
	{"/", "docs", "/docs"},
	{"/docs", "", "/docs"}
	};
	char					out[VFS_PATH_MAX];
	size_t					i;

	i = 0;
	while (i < sizeof(t) / sizeof(t[0]))
	{
		h_eq_i64("code", con_path_join(t[i].cwd, t[i].arg, out, sizeof(out)),
			0);
		h_eq_str(t[i].arg, out, t[i].want);
		i++;
	}
}

static void	chemin_trop_long(void)
{
	char	big[VFS_PATH_MAX * 2];
	char	out[VFS_PATH_MAX];

	memset(big, 'a', sizeof(big));
	big[sizeof(big) - 1] = '\0';
	h_eq_i64("composant trop long", con_path_join("/", big, out, sizeof(out)),
		E_RANGE);
	h_eq_i64("tampon minuscule", con_path_join("/", "a", out, 1), E_INVAL);
	h_eq_i64("tampon trop court", con_path_join("/", "abc", out, 3), E_RANGE);
}

static void	binaire_et_argument(void)
{
	char	out[VFS_PATH_MAX];

	h_eq_i64("dans bin", con_path_in_bin("/system/bin/hello"), 1);
	h_eq_i64("bin seul", con_path_in_bin("/system/bin"), 0);
	h_eq_i64("sous-dossier", con_path_in_bin("/system/bin/a/b"), 0);
	h_eq_i64("voisin binx", con_path_in_bin("/system/binx/hello"), 0);
	h_eq_i64("etc", con_path_in_bin("/system/etc/os-release"), 0);
	h_eq_i64("docs", con_path_in_bin("/docs/notes.txt"), 0);
	h_eq_i64("mot", con_confine_arg("/", "bonjour", out, sizeof(out)), 0);
	h_eq_str("mot inchange", out, "bonjour");
	h_eq_i64("sous systeme", con_confine_arg("/system/bin",
			"../etc/os-release", out, sizeof(out)), 0);
	h_eq_str("absolu", out, "/system/etc/os-release");
	h_eq_i64("hors systeme",
		con_confine_arg("/", "/docs/notes.txt", out, sizeof(out)), E_ACCES);
	h_eq_i64("system puis remontee",
		con_confine_arg("/", "/system/../docs/x", out, sizeof(out)), E_ACCES);
	h_eq_i64("systemd", con_confine_arg("/", "/systemd", out, sizeof(out)),
		E_ACCES);
	h_eq_i64("racine via point point",
		con_confine_arg("/", "..", out, sizeof(out)), E_ACCES);
	h_eq_i64("reste dans systeme",
		con_confine_arg("/system/etc", "..", out, sizeof(out)), 0);
	h_eq_str("dossier systeme", out, "/system");
	h_eq_i64("controle dans un mot",
		con_confine_arg("/", "a\nb", out, sizeof(out)), E_INVAL);
	h_eq_i64("argument nul", con_confine_arg("/", NULL, out, sizeof(out)),
		E_INVAL);
}

static void	chemin_octets(void)
{
	char	out[VFS_PATH_MAX];

	h_eq_i64("controle", con_path_join("/", "a\nb", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("del", con_path_join("/", "a\177", out, sizeof(out)), E_INVAL);
	h_eq_i64("echappement", con_path_join("/", "\033[2J", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("c1", con_path_join("/", "\302\205", out, sizeof(out)), E_INVAL);
	h_eq_i64("sur-long", con_path_join("/", "\300\257", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("demi-codet", con_path_join("/", "\355\240\200", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("tronque", con_path_join("/", "\342\202", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("octet ff", con_path_join("/", "\377", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("continuation fausse",
		con_path_join("/", "\303\050", out, sizeof(out)), E_INVAL);
	h_eq_i64("au-dela", con_path_join("/", "\364\220\200\200", out, sizeof(out)),
		E_INVAL);
	h_eq_i64("e accent", con_path_join("/docs", "\303\251", out, sizeof(out)),
		0);
	h_eq_str("e accent garde", out, "/docs/\303\251");
	h_eq_i64("limite u+00a0", con_path_join("/", "\302\240", out, sizeof(out)),
		0);
	h_eq_str("u+00a0 garde", out, "/\302\240");
	h_eq_i64("limite u+10ffff",
		con_path_join("/", "\364\217\277\277", out, sizeof(out)), 0);
}

int	main(void)
{
	h_begin("e15_analyse");
	h_run("decoupage", decoupage);
	h_run("decoupage_invalide", decoupage_invalide);
	h_run("chemins", chemins);
	h_run("chemin_trop_long", chemin_trop_long);
	h_run("chemin_octets", chemin_octets);
	h_run("binaire_et_argument", binaire_et_argument);
	return (h_end());
}
