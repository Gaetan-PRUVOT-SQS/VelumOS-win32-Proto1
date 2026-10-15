#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fsys.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static int64_t	op(const char *path, uint64_t flags)
{
	return (sys_open(fake_args((uint64_t)path, strlen(path), flags, 0)));
}

static void	noms(void)
{
	t_vstat	st;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("fichier", vst_put("/d/f.txt", "0123456789", 10), 0);
	fake_proc()->flags = PF_FSWRITE;
	h_eq_i64("mkdir", sys_mkdir(fake_args((uint64_t)"/d/rep", 6, 0, 0)), 0);
	h_eq_i64("rename", sys_rename(fake_args((uint64_t)"/d/f.txt", 8,
				(uint64_t)"/d/rep/g.txt", 12)), 0);
	h_eq_i64("stat", sys_stat(fake_args((uint64_t)"/d/rep/g.txt", 12,
				(uint64_t)(&st), 0)), 0);
	h_eq_i64("stat sortie nulle", sys_stat(fake_args((uint64_t)"/d/rep", 6, 0,
				0)), E_FAULT);
}

static void	liste_dossier(void)
{
	t_dirent	*out;
	int64_t		h;

	out = calloc(70, sizeof(t_dirent));
	h = op("/d", O_RDONLY | O_DIRECTORY);
	h_eq_i64("readdir", sys_readdir(fake_args(h, (uint64_t)out, 1000, 0)), 1);
	h_eq_str("nom", out[0].name, "rep");
	h_eq_i64("readdir fin", sys_readdir(fake_args(h, (uint64_t)out, 5, 0)),
		0);
	h_eq_i64("readdir sortie noyau", sys_readdir(fake_args(h, FAKE_KADDR, 5,
				0)), E_FAULT);
	h_eq_i64("read sur dossier", sys_read(fake_args(h, (uint64_t)out, 5, 0)),
		E_BADF);
	h_eq_i64("fsync dossier", sys_fsync(fake_args(h, 0, 0, 0)), 0);
	fake_close((t_handle)h);
	h_eq_i64("unlink", sys_unlink(fake_args((uint64_t)"/d/rep/g.txt", 12, 0,
				0)), 0);
	h_eq_i64("rmdir", sys_unlink(fake_args((uint64_t)"/d/rep", 6, 0, 0)), 0);
	free(out);
}

static void	plafond_256(void)
{
	int64_t		h[257];
	uint64_t	live;
	int			i;

	live = fake_heap_live();
	vst_put("/d/q", "q", 1);
	i = -1;
	while (++i < 256)
		h[i] = op("/d/q", O_RDONLY);
	h_true(h[0] > 0 && h[255] > 0, "256 ouverts");
	h_eq_i64("257e : E_MFILE", op("/d/q", O_RDONLY), E_MFILE);
	fake_close((t_handle)h[10]);
	h[10] = op("/d/q", O_RDONLY);
	h_true(h[10] > 0, "place libérée");
	i = -1;
	while (++i < 256)
		fake_close((t_handle)h[i]);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_true(fake_heap_live() < live, "aucune fuite après fermeture");
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/sys_dir");
	h_run("mkdir, rename, stat", noms);
	h_run("readdir et suppression", liste_dossier);
	h_run("plafond de 256 fichiers", plafond_256);
	return (h_end());
}
