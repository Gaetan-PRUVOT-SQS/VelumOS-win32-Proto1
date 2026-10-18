#include "velum/libk.h"
#include "harness.h"
#include "velum/err.h"
#include "fichiers_model.h"

static void	nom_partitions(void)
{
	char	long_name[VFS_NAME_MAX + 2];

	h_true(ff_name_valid("notes-reunion.txt"), "nom ordinaire");
	h_true(ff_name_valid("é accentué"), "octets hauts UTF-8 acceptés");
	h_true(!ff_name_valid(NULL), "pointeur nul refusé");
	h_true(!ff_name_valid(""), "vide refusé");
	h_true(!ff_name_valid("."), "point refusé");
	h_true(!ff_name_valid(".."), "deux points refusés");
	h_true(!ff_name_valid("a/b"), "séparateur refusé");
	h_true(!ff_name_valid("a\tb"), "contrôle refusé");
	h_true(!ff_name_valid("a\x7f"), "DEL refusé");
	h_true(!ff_name_valid("/"), "barre seule refusée");
	h_true(ff_name_valid("..."), "trois points acceptés");
	h_true(ff_name_valid(".cache"), "nom caché accepté");
	memset(long_name, 'x', sizeof(long_name));
	long_name[VFS_NAME_MAX] = '\0';
	h_true(ff_name_valid(long_name), "255 octets : limite acceptée");
	long_name[VFS_NAME_MAX] = 'x';
	long_name[VFS_NAME_MAX + 1] = '\0';
	h_true(!ff_name_valid(long_name), "256 octets : refusé");
}

static void	controles(void)
{
	char		s[3];
	unsigned	c;

	s[2] = '\0';
	c = 1;
	while (c < 0x20)
	{
		s[0] = 'a';
		s[1] = (char)c;
		h_eq_u64("contrôle dans un nom", ff_name_valid(s) ? c : 0, 0);
		s[0] = (char)c;
		s[1] = '\0';
		h_eq_u64("contrôle seul", ff_name_valid(s) ? c : 0, 0);
		c++;
	}
	s[0] = '\x7f';
	s[1] = '\0';
	h_eq_u64("DEL seul", ff_name_valid(s), 0);
}

static void	titre(void)
{
	h_eq_str("nul", ff_dir_title(NULL), "Ordinateur");
	h_eq_str("vide", ff_dir_title(""), "Ordinateur");
	h_eq_str("racine", ff_dir_title("/"), "Ordinateur");
	h_eq_str("relatif", ff_dir_title("data"), "Ordinateur");
	h_eq_str("système", ff_dir_title("/system"), "system");
	h_eq_str("feuille", ff_dir_title("/data/docs"), "docs");
}

static void	jointure(void)
{
	char	out[VFS_PATH_MAX];

	h_eq_i64("racine + nom", ff_path_join(out, sizeof(out), "/", "data"), 0);
	h_eq_str("racine + nom : chemin", out, "/data");
	h_eq_i64("dossier + nom", ff_path_join(out, sizeof(out), "/data", "a"), 0);
	h_eq_str("dossier + nom : chemin", out, "/data/a");
	h_eq_i64("nom invalide", ff_path_join(out, sizeof(out), "/", "a/b"),
		E_INVAL);
	h_eq_i64("dossier relatif", ff_path_join(out, sizeof(out), "data", "a"),
		E_INVAL);
	h_eq_i64("tampon trop petit", ff_path_join(out, 9, "/data", "abc"),
		E_RANGE);
	h_eq_i64("tampon juste", ff_path_join(out, 10, "/data", "abc"), 0);
}

static void	jointure_limite_chemin(void)
{
	char	dir[VFS_PATH_MAX];
	char	out[2 * VFS_PATH_MAX];
	size_t	i;

	dir[0] = '/';
	i = 1;
	while (i < VFS_PATH_MAX - 2)
		dir[i++] = 'd';
	dir[i] = '\0';
	h_eq_i64("chemin résultant de 256 octets refusé", ff_path_join(out, sizeof(out),
			dir, "a"), E_RANGE);
	dir[VFS_PATH_MAX - 3] = '\0';
	h_eq_i64("chemin résultant de 255 octets accepté", ff_path_join(out, sizeof(out),
			dir, "a"), 0);
	h_eq_u64("longueur du résultat", strlen(out), VFS_PATH_MAX - 1);
}

static void	parent(void)
{
	char	p[VFS_PATH_MAX];

	strlcpy(p, "/data/docs/x", sizeof(p));
	h_eq_i64("parent profond", ff_path_parent(p), 0);
	h_eq_str("parent profond : chemin", p, "/data/docs");
	strlcpy(p, "/data", sizeof(p));
	h_eq_i64("parent de /data", ff_path_parent(p), 0);
	h_eq_str("parent de /data : racine", p, "/");
	strlcpy(p, "/", sizeof(p));
	h_eq_i64("parent de la racine refusé", ff_path_parent(p), E_INVAL);
	h_eq_str("racine inchangée", p, "/");
	strlcpy(p, "relatif", sizeof(p));
	h_eq_i64("chemin relatif refusé", ff_path_parent(p), E_INVAL);
	strlcpy(p, "/data//", sizeof(p));
	h_eq_i64("barres finales", ff_path_parent(p), 0);
	h_eq_str("barres finales : racine", p, "/");
	h_true(ff_path_is_root("/") && !ff_path_is_root("/a"), "racine reconnue");
}

int	main(void)
{
	h_begin("e11/chemins");
	h_run("noms: partitions et limites", nom_partitions);
	h_run("noms: contrôles 0x01-0x1F et DEL", controles);
	h_run("titre: chemin vide ou racine", titre);
	h_run("jointure: cas et limites", jointure);
	h_run("jointure: limite de VFS_PATH_MAX", jointure_limite_chemin);
	h_run("parent: cas", parent);
	return (h_end());
}
