#include <string.h>
#include "harness.h"
#include "vfs_int.h"
#include "velum/err.h"

static void	partition_valides(void)
{
	char	o[VFS_PATH_MAX];

	h_true(vpath_norm("/", o) == 0 && strcmp(o, "/") == 0, "racine");
	h_true(vpath_norm("//a", o) == 0 && strcmp(o, "/a") == 0, "double /");
	h_true(vpath_norm("/a/./b/", o) == 0 && strcmp(o, "/a/b") == 0, ". final");
	h_true(vpath_norm("/a/../..", o) == 0 && strcmp(o, "/") == 0,
		".. sans sortir de la racine");
	h_true(vpath_norm("/../../../etc", o) == 0 && strcmp(o, "/etc") == 0,
		".. en boucle");
	h_true(vpath_norm("//a/./b/../c", o) == 0 && strcmp(o, "/a/c") == 0,
		"mélange");
	h_true(vpath_norm("/é/日本/😀", o) == 0
		&& strcmp(o, "/é/日本/😀") == 0, "UTF-8 2, 3 et 4 octets");
	h_true(vpath_norm("/...", o) == 0 && strcmp(o, "/...") == 0, "... nom");
}

static void	partition_invalides(void)
{
	char	o[VFS_PATH_MAX];

	h_eq_i64("vide", vpath_norm("", o), E_INVAL);
	h_eq_i64("relatif", vpath_norm("a/../..", o), E_INVAL);
	h_eq_i64("relatif point", vpath_norm("./a", o), E_INVAL);
	h_eq_i64("contrôle C0", vpath_norm("/a\nb", o), E_INVAL);
	h_eq_i64("DEL", vpath_norm("/a\x7f", o), E_INVAL);
	h_eq_i64("contrôle C1", vpath_norm("/a\xc2\x85", o), E_INVAL);
	h_eq_i64("continuation seule", vpath_norm("/\x80", o), E_INVAL);
	h_eq_i64("surlong", vpath_norm("/\xc0\xaf", o), E_INVAL);
	h_eq_i64("surlong 3", vpath_norm("/\xe0\x80\xaf", o), E_INVAL);
	h_eq_i64("surrogate", vpath_norm("/\xed\xa0\x80", o), E_INVAL);
	h_eq_i64("au-delà 10FFFF", vpath_norm("/\xf4\x90\x80\x80", o), E_INVAL);
	h_eq_i64("tronqué", vpath_norm("/\xe6\x97", o), E_INVAL);
	h_eq_i64("F5", vpath_norm("/\xf5\x80\x80\x80", o), E_INVAL);
}

static void	limites_longueur(void)
{
	char	in[VFS_PATH_MAX + 8];
	char	o[VFS_PATH_MAX];

	memset(in, 'a', sizeof(in));
	in[0] = '/';
	in[255] = '\0';
	h_eq_i64("255 octets", vpath_norm(in, o), 0);
	h_eq_u64("255 conservés", strlen(o), 255);
	in[255] = 'a';
	in[256] = '\0';
	h_eq_i64("256 octets", vpath_norm(in, o), E_RANGE);
	in[200] = '/';
	in[255] = '\0';
	h_eq_i64("deux composants", vpath_norm(in, o), 0);
}

static void	decoupe_et_noms(void)
{
	char		comp[VFS_NAME_MAX + 1];
	char		par[VFS_PATH_MAX];
	const char	*name;
	const char	*p;

	p = vpath_next("a//bc/", comp);
	h_true(p && strcmp(comp, "a") == 0, "composant 1");
	p = vpath_next(p, comp);
	h_true(p && strcmp(comp, "bc") == 0, "composant 2");
	h_true(vpath_next(p, comp) == NULL, "fin");
	h_true(vpath_split("x/y/z", par, &name) == 0 && strcmp(par, "x/y") == 0
		&& strcmp(name, "z") == 0, "split");
	h_true(vpath_split("z", par, &name) == 0 && par[0] == '\0', "split seul");
	h_eq_i64("split vide", vpath_split("", par, &name), E_INVAL);
	h_true(vname_eq("ReadMe.TXT", "readme.txt") && !vname_eq("a", "ab"),
		"comparaison sans casse");
}

int	main(void)
{
	h_begin("a12/path");
	h_run("partitions valides", partition_valides);
	h_run("partitions invalides", partition_invalides);
	h_run("limites de longueur", limites_longueur);
	h_run("découpe et noms", decoupe_et_noms);
	return (h_end());
}
