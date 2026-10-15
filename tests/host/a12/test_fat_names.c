#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	noms_longs(void)
{
	t_vstat	st;
	t_vfile	*f;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("nom long", vst_put("/d/Un nom de fichier vraiment long.txt",
			"L", 1), 0);
	h_eq_i64("casse différente", vst_same(
			"/d/UN NOM DE FICHIER VRAIMENT LONG.TXT", "L", 1), 0);
	h_eq_i64("minuscules 8.3", vst_put("/d/readme.txt", "r", 1), 0);
	h_eq_i64("par nom court", vfs_stat("/d/README.TXT", &st), 0);
	h_eq_i64("doublon O_EXCL", vfs_open("/d/README.TXT", O_CREAT | O_EXCL
			| O_WRONLY, &f), E_EXIST);
	h_eq_i64("UTF-8", vst_put("/d/Été 日本 😀.txt", "u", 1), 0);
	h_eq_i64("UTF-8 relu", vst_same("/d/été 日本 😀.TXT", "u", 1), 0);
	h_eq_i64("13 unités", vst_put("/d/abcdefghijklm", "a", 1), 0);
	h_eq_i64("26 unités", vst_put("/d/abcdefghijklmnopqrstuvwxyz", "b", 1),
		0);
}

static void	composant_limite(void)
{
	char	name[300];

	memset(name, 'n', sizeof(name));
	memcpy(name, "/d/", 3);
	name[255] = '\0';
	h_eq_i64("composant de 252", vst_put(name, "c", 1), 0);
	h_eq_i64("relu 252", vst_same(name, "c", 1), 0);
	name[255] = 'n';
	name[256] = '\0';
	h_eq_i64("chemin de 256", vst_put(name, "c", 1), E_RANGE);
}

static void	noms_courts(void)
{
	char		path[64];
	t_vstat		st;
	uint32_t	i;

	i = 0;
	while (i < 8)
	{
		snprintf(path, sizeof(path), "/d/Long name %u.txt", i);
		h_eq_i64("création série", vst_put(path, "s", 1), 0);
		i++;
	}
	h_eq_i64("~1", vfs_stat("/d/LONGNA~1.TXT", &st), 0);
	h_eq_i64("~4", vfs_stat("/d/LONGNA~4.TXT", &st), 0);
	h_eq_i64("~5 remplacé par hachage", vfs_stat("/d/LONGNA~5.TXT", &st),
		E_NOENT);
	h_eq_i64("majuscules 8.3 sans LFN", vst_put("/d/UPPER.TXT", "U", 1), 0);
	h_eq_i64("noms courts uniques", fake_dup_snames(g_b), 0);
	h_eq_i64("nom ':'", vst_put("/d/a:b", "x", 1), E_INVAL);
	h_eq_i64("nom '*'", vst_put("/d/a*", "x", 1), E_INVAL);
	h_eq_i64("point final", vst_put("/d/fin.", "x", 1), E_INVAL);
	h_eq_i64("espace initial", vst_put("/d/ x", "x", 1), E_INVAL);
	h_eq_i64("antislash", vst_put("/d/a\\b", "x", 1), E_INVAL);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_names");
	h_run("noms longs et UTF-8", noms_longs);
	h_run("composant de 252, chemin de 256", composant_limite);
	h_run("noms courts générés et noms refusés", noms_courts);
	return (h_end());
}
