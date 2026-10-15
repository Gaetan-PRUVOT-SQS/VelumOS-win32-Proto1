#include "vfs_int.h"
#include "velum/err.h"
#include "velum/heap.h"

int	vfs_mkdir(const char *path, uint32_t mode)
{
	t_vres	r;
	t_vnode	n;
	int		rc;

	(void)mode;
	rc = vfs_ns_begin(path, &r);
	if (rc == E_BUSY)
		return (E_EXIST);
	if (rc < 0)
		return (rc);
	mutex_lock(&r.mnt->lock);
	rc = r.mnt->ops->lookup(r.mnt->fs, r.rel, &n);
	if (rc == 0)
		rc = E_EXIST;
	else if (rc == E_NOENT)
		rc = r.mnt->ops->create(r.mnt->fs, r.rel, S_TYPE_DIR, &n);
	mutex_unlock(&r.mnt->lock);
	vfs_mnt_put(r.mnt);
	return (rc);
}

static int	readall_fill(t_vfile *f, uint8_t *buf, uint64_t size)
{
	uint64_t	done;
	int64_t		n;

	done = 0;
	while (done < size)
	{
		n = vfs_pread(f, buf + done, size - done, done);
		if (n < 0)
			return ((int)n);
		if (n == 0)
			return (E_IO);
		done += (uint64_t)n;
	}
	return (0);
}

static int	readall_alloc(t_vfile *f, uint8_t **b, uint64_t *size)
{
	t_vstat	st;
	int		rc;

	rc = vfs_fstat(f, &st);
	if (rc < 0)
		return (rc);
	if (st.mode & S_TYPE_DIR)
		return (E_ISDIR);
	if (st.size > VFS_READALL_MAX)
		return (E_RANGE);
	*b = kmalloc_tag(st.size + 1, HEAP_FS);
	if (*b == NULL)
		return (E_NOMEM);
	*size = st.size;
	return (0);
}

int	vfs_read_all(const char *path, void **buf, size_t *size)
{
	t_vfile		*f;
	uint8_t		*b;
	uint64_t	n;
	int			rc;

	if (buf == NULL || size == NULL)
		return (E_INVAL);
	rc = vfs_open(path, O_RDONLY, &f);
	if (rc < 0)
		return (rc);
	b = NULL;
	rc = readall_alloc(f, &b, &n);
	if (rc == 0)
		rc = readall_fill(f, b, n);
	vfs_close(f);
	if (rc < 0)
	{
		kfree(b);
		return (rc);
	}
	*buf = b;
	*size = n;
	return (0);
}
