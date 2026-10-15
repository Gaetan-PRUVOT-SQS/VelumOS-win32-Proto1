#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fsys.h"
#include "velum/err.h"

static t_fakeblk	*g_b;
static int64_t		g_fd;

static int64_t	op(const char *path, uint64_t flags)
{
	return (sys_open(fake_args((uint64_t)path, strlen(path), flags, 0)));
}

static void	lecture_positions(void)
{
	char	buf[32];
	t_vstat	st;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	h_eq_i64("fichier", vst_put("/d/f.txt", "0123456789", 10), 0);
	fake_proc()->flags = PF_FSWRITE;
	g_fd = op("/d/f.txt", O_RDWR);
	h_eq_i64("read", sys_read(fake_args(g_fd, (uint64_t)buf, 4, 0)), 4);
	h_eq_i64("read suite", sys_read(fake_args(g_fd, (uint64_t)buf + 4, 32, 0)),
		6);
	h_true(memcmp(buf, "0123456789", 10) == 0, "contenu lu");
	h_eq_i64("pwrite", sys_pwrite(fake_args(g_fd, (uint64_t)"AB", 2, 3)), 2);
	h_eq_i64("pread", sys_pread(fake_args(g_fd, (uint64_t)buf, 4, 2)), 4);
	h_true(memcmp(buf, "2AB5", 4) == 0, "pread contenu");
	h_eq_i64("seek", sys_seek(fake_args(g_fd, 1, SEEK_SET_, 0)), 1);
	h_eq_i64("whence fou", sys_seek(fake_args(g_fd, 1, 1ull << 35, 0)),
		E_INVAL);
	h_true(sys_fstat(fake_args(g_fd, (uint64_t)(&st), 0, 0)) == 0
		&& st.size == 10, "fstat");
}

static void	pointeurs_refuses(void)
{
	h_eq_i64("read tampon nul", sys_read(fake_args(g_fd, 0, 4, 0)), E_FAULT);
	h_eq_i64("read tampon noyau", sys_read(fake_args(g_fd, FAKE_KADDR, 4, 0)),
		E_FAULT);
	h_eq_i64("write au bord haut", sys_write(fake_args(g_fd,
				0x7ffffffff000ull, 1 << 20, 0)), E_FAULT);
	h_eq_i64("fstat sortie noyau", sys_fstat(fake_args(g_fd, FAKE_KADDR, 0, 0)),
		E_FAULT);
	fake_close((t_handle)g_fd);
}

static void	droits_handle(void)
{
	char	buf[32];
	int64_t	h;
	int64_t	r;

	h = op("/d/f.txt", O_RDONLY);
	h_eq_i64("write sans HR_WRITE", sys_write(fake_args(h, (uint64_t)"x", 1,
				0)), E_PERM);
	h_eq_i64("truncate sans HR_WRITE", sys_truncate(fake_args(h, 0, 0, 0)),
		E_PERM);
	r = sys_readdir(fake_args(h, (uint64_t)buf, 1, 0));
	h_eq_i64("readdir sur fichier", r, E_BADF);
	fake_close((t_handle)h);
	h_eq_i64("handle 0", sys_read(fake_args(0, (uint64_t)buf, 1, 0)), E_BADF);
	h_eq_i64("handle 2^32", sys_read(fake_args(1ull << 32, (uint64_t)buf, 1,
				0)), E_BADF);
	h_eq_i64("handle fermé", sys_read(fake_args(h, (uint64_t)buf, 1, 0)),
		E_BADF);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/sys_rw");
	h_run("lecture, écriture, positions", lecture_positions);
	h_run("tampons refusés", pointeurs_refuses);
	h_run("droits de handle", droits_handle);
	return (h_end());
}
