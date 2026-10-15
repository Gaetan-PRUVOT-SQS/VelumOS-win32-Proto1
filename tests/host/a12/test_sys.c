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

static void	ouverture_privileges(void)
{
	int64_t	h;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	fakefat_mount(g_b, "/d");
	vst_put("/d/f.txt", "0123456789", 10);
	fake_proc()->flags = 0;
	h = op("/d/f.txt", O_RDONLY);
	h_true(h > 0 && fake_close((t_handle)h) == 0, "lecture sans PF_FSWRITE");
	h_eq_i64("O_WRONLY sans PF", op("/d/f.txt", O_WRONLY), E_PERM);
	h_eq_i64("O_RDWR sans PF", op("/d/f.txt", O_RDWR), E_PERM);
	h_eq_i64("O_CREAT sans PF", op("/d/n", O_CREAT), E_PERM);
	h_eq_i64("O_TRUNC sans PF", op("/d/f.txt", O_TRUNC | O_RDWR), E_PERM);
	h_eq_i64("mkdir sans PF", sys_mkdir(fake_args((uint64_t)"/d/k", 4, 0, 0)),
		E_PERM);
	fake_proc()->flags = PF_FSWRITE;
	h = op("/d/f.txt", O_RDWR);
	h_true(h > 0 && fake_close((t_handle)h) == 0, "O_RDWR avec PF");
}

static void	ouverture_pointeurs(void)
{
	h_eq_i64("chemin nul", sys_open(fake_args(0, 4, 0, 0)), E_FAULT);
	h_eq_i64("chemin noyau", sys_open(fake_args(FAKE_KADDR, 4, 0, 0)),
		E_FAULT);
	h_eq_i64("longueur 0", sys_open(fake_args((uint64_t)"/d", 0, 0, 0)),
		E_INVAL);
	h_eq_i64("longueur 256", sys_open(fake_args((uint64_t)"/d", 256, 0, 0)),
		E_RANGE);
	h_eq_i64("longueur géante", sys_open(fake_args((uint64_t)"/d",
				1ull << 40, 0, 0)), E_RANGE);
	h_eq_i64("NUL interne", sys_open(fake_args((uint64_t)"/d\0/f.txt", 9, 0,
				0)), E_INVAL);
	h_eq_i64("drapeaux 64 bits", op("/d/f.txt", 1ull << 33), E_INVAL);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/sys");
	h_run("ouverture : privilèges", ouverture_privileges);
	h_run("ouverture : pointeurs et longueurs", ouverture_pointeurs);
	return (h_end());
}
