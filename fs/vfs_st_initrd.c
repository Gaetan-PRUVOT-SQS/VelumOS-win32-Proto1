#include "vfs_st.h"
#include "velum/heap.h"

static int	build(t_vbuf *b)
{
	int	rc;

	rc = vst_cpio_add(b, ".", 040755, NULL);
	if (rc == 0)
		rc = vst_cpio_add(b, "system", 040755, NULL);
	if (rc == 0)
		rc = vst_cpio_add(b, "system/bin/hello", 0100644, "bonjour");
	if (rc == 0)
		rc = vst_cpio_add(b, "docs/notes/a.txt", 0100644, "abc");
	if (rc == 0)
		rc = vst_cpio_add(b, "TRAILER!!!", 0, NULL);
	return (rc);
}

static int	count_dir(const char *path)
{
	t_vfile		*f;
	t_dirent	d;
	int			n;

	if (vfs_open(path, O_RDONLY | O_DIRECTORY, &f) < 0)
		return (-1);
	n = 0;
	while (vfs_readdir(f, &d) == 1)
		n++;
	vfs_close(f);
	return (n);
}

int	vst_initrd(void)
{
	t_vbuf	b;
	int		fails;

	b.cap = 1024;
	b.len = 0;
	b.p = kmalloc_tag(b.cap, HEAP_FS);
	if (b.p == NULL)
		return (1);
	fails = vst_check(build(&b) == 0
			&& vfs_mount_initrd("/.vst", b.p, b.len) == 0, "montage initrd");
	if (fails == 0)
	{
		fails += vst_check(vst_same("/.vst/system/bin/hello", "bonjour", 7)
				== 0, "lecture initrd");
		fails += vst_check(count_dir("/.vst") == 2, "liste initrd");
		fails += vst_check(count_dir("/.vst/docs") == 1, "dossier implicite");
		fails += vst_check(vfs_umount("/.vst") == 0, "démontage initrd");
	}
	kfree(b.p);
	return (fails);
}
