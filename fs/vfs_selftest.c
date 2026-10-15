#include "vfs_st.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/libk.h"

int	vst_check(int ok, const char *what)
{
	if (ok)
		return (0);
	klog_err("vfs: autotest %s KO", what);
	return (1);
}

int	vst_put(const char *path, const void *data, uint64_t len)
{
	t_vfile	*f;
	int64_t	w;
	int		rc;

	rc = vfs_open(path, O_CREAT | O_TRUNC | O_WRONLY, &f);
	if (rc < 0)
		return (rc);
	w = vfs_write(f, data, len);
	rc = vfs_fsync(f);
	vfs_close(f);
	if (w < 0)
		return ((int)w);
	if ((uint64_t)w != len)
		return (E_IO);
	return (rc);
}

int	vst_same(const char *path, const void *data, uint64_t len)
{
	void	*buf;
	size_t	n;
	int		rc;

	rc = vfs_read_all(path, &buf, &n);
	if (rc < 0)
		return (rc);
	rc = E_IO;
	if (n == len && memcmp(buf, data, len) == 0)
		rc = 0;
	kfree(buf);
	return (rc);
}

static int	st_basics(void)
{
	char	out[VFS_PATH_MAX];
	t_vstat	st;
	t_vfile	*f;
	int		fails;

	fails = vst_check(vpath_norm("/a/../..//b/./c", out) == 0
			&& strcmp(out, "/b/c") == 0, "chemin ..");
	fails += vst_check(vpath_norm("rel", out) == E_INVAL, "chemin relatif");
	fails += vst_check(vpath_norm("/a\x01", out) == E_INVAL, "chemin ctrl");
	fails += vst_check(vfs_stat("/", &st) == 0 && (st.mode & S_TYPE_DIR),
			"racine");
	fails += vst_check(vfs_open("/vfs-ro", O_CREAT | O_WRONLY, &f) == E_PERM,
			"racine en lecture seule");
	fails += vst_check(vfs_mkdir("/vfs-ro", 0) == E_PERM, "mkdir racine");
	return (fails);
}

int	vfs_selftest(void)
{
	t_heap_stats	before;
	t_heap_stats	after;
	int				fails;

	heap_get_stats(&before);
	fails = st_basics();
	fails += vst_initrd();
	fails += vst_data();
	heap_get_stats(&after);
	fails += vst_check(before.allocs_live[HEAP_FS]
			== after.allocs_live[HEAP_FS], "fuite HEAP_FS");
	return (fails);
}
