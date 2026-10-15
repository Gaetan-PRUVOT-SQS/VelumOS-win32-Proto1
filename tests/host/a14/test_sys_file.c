#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static int64_t	g_ret;

static void	file_rw(void)
{
	const char	*path = "/a/bc";
	char		buf[8];

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("open", v_open(path, O_RDWR | O_CREAT, 0x1a4), g_ret);
	sys_is(SYS_OPEN, sys_u(path), 5, O_RDWR | O_CREAT);
	sys_is_hi(0x1a4, 0, 0);
	h_eq_i64("read", v_read(0x61, buf, 8), g_ret);
	sys_is(SYS_READ, 0x61, sys_u(buf), 8);
	h_eq_i64("write", v_write(0x62, buf, 7), g_ret);
	sys_is(SYS_WRITE, 0x62, sys_u(buf), 7);
	h_eq_i64("pread", v_pread(0x63, buf, 6, 0x1000), g_ret);
	sys_is(SYS_PREAD, 0x63, sys_u(buf), 6);
	sys_is_hi(0x1000, 0, 0);
	h_eq_i64("pwrite", v_pwrite(0x64, buf, 5, 0x2000), g_ret);
	sys_is(SYS_PWRITE, 0x64, sys_u(buf), 5);
	sys_is_hi(0x2000, 0, 0);
}

static void	file_meta(void)
{
	const char	*path = "/a/bc";
	t_vstat		st;
	t_dirent	ent[2];

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("seek", v_seek(0x71, 4096, SEEK_CUR_), g_ret);
	sys_is(SYS_SEEK, 0x71, 4096, SEEK_CUR_);
	h_eq_i64("stat", v_stat(path, &st), g_ret);
	sys_is(SYS_STAT, sys_u(path), 5, sys_u(&st));
	h_eq_i64("fstat", v_fstat(0x72, &st), g_ret);
	sys_is(SYS_FSTAT, 0x72, sys_u(&st), 0);
	h_eq_i64("readdir", v_readdir(0x73, ent, 2), g_ret);
	sys_is(SYS_READDIR, 0x73, sys_u(ent), 2);
	h_eq_i64("mkdir", v_mkdir(path, 0x1ed), g_ret);
	sys_is(SYS_MKDIR, sys_u(path), 5, 0x1ed);
}

static void	file_names(void)
{
	const char	*old_path = "/old";
	const char	*new_path = "/new/x";

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("unlink", v_unlink(old_path), g_ret);
	sys_is(SYS_UNLINK, sys_u(old_path), 4, 0);
	h_eq_i64("rename", v_rename(old_path, new_path), g_ret);
	sys_is(SYS_RENAME, sys_u(old_path), 4, sys_u(new_path));
	sys_is_hi(6, 0, 0);
	h_eq_i64("fsync", v_fsync(0x81), g_ret);
	sys_is(SYS_FSYNC, 0x81, 0, 0);
	h_eq_i64("truncate", v_truncate(0x82, 0x123456789ull), g_ret);
	sys_is(SYS_TRUNCATE, 0x82, 0x123456789ull, 0);
}

static void	file_partitions(void)
{
	char	buf[4];

	fake_reset();
	g_fsys.ret = 0x123456789ll;
	h_eq_i64("seek offset negatif",
		v_seek(1, -5, SEEK_END_), 0x123456789ll);
	sys_is(SYS_SEEK, 1, (uint64_t)-5ll, SEEK_END_);
	h_eq_i64("read retour 64 bits", v_read(1, buf, 4), 0x123456789ll);
	h_eq_i64("pread decalage 64 bits", v_pread(1, buf, 4, 0x123456789abcull),
		0x123456789ll);
	sys_is_hi(0x123456789abcull, 0, 0);
	h_eq_i64("open retour 64 bits", v_open("/x", O_RDONLY, 0), 0x123456789ll);
	h_eq_i64("write retour 64 bits", v_write(1, buf, 4), 0x123456789ll);
}

int	main(void)
{
	h_begin("a14/sys_file");
	g_ret = 11;
	h_run("vfile/exigence : open, lecture, ecriture, retour positif", file_rw);
	h_run("vfile/exigence : seek, stat, readdir, mkdir, retour positif",
		file_meta);
	h_run("vfile/exigence : unlink, rename, fsync, truncate, retour positif",
		file_names);
	g_ret = E_NOENT;
	h_run("vfile/exigence : open, lecture, ecriture, retour negatif", file_rw);
	h_run("vfile/exigence : seek, stat, readdir, mkdir, retour negatif",
		file_meta);
	h_run("vfile/exigence : unlink, rename, fsync, truncate, retour negatif",
		file_names);
	h_run("vfile/valeur limite : decalages et retours 64 bits",
		file_partitions);
	return (h_end());
}
