#include "vfs_int.h"
#include "velum/err.h"
#include "velum/libk.h"

int64_t	vfs_pread(struct s_vfile *f, void *buf, size_t n, uint64_t off)
{
	t_vio	io;

	if (buf == NULL && n)
		return (E_INVAL);
	io.dst = buf;
	io.src = NULL;
	io.len = n;
	io.off = off;
	return (vfs_xfer(f, &io, 0, 0));
}

int64_t	vfs_pwrite(struct s_vfile *f, const void *b, size_t n, uint64_t off)
{
	t_vio	io;

	if (b == NULL && n)
		return (E_INVAL);
	io.dst = NULL;
	io.src = b;
	io.len = n;
	io.off = off;
	return (vfs_xfer(f, &io, 1, 0));
}

static int	seek_target(t_vfile *f, int64_t off, int whence, int64_t *out)
{
	int64_t	base;

	if (whence == SEEK_SET_)
		base = 0;
	else if (whence == SEEK_CUR_)
		base = (int64_t)f->pos;
	else if (whence == SEEK_END_)
	{
		mutex_lock(&f->mnt->lock);
		base = (int64_t)f->node->size;
		mutex_unlock(&f->mnt->lock);
	}
	else
		return (E_INVAL);
	if (__builtin_add_overflow(base, off, out) || *out < 0)
		return (E_INVAL);
	return (0);
}

int64_t	vfs_seek(struct s_vfile *f, int64_t off, int whence)
{
	int64_t	np;
	int		rc;

	if (f == NULL)
		return (E_BADF);
	if ((f->node->mode & S_TYPE_DIR) && (whence != SEEK_SET_ || off != 0))
		return (E_INVAL);
	mutex_lock(&f->lock);
	rc = seek_target(f, off, whence, &np);
	if (rc == 0)
		f->pos = (uint64_t)np;
	mutex_unlock(&f->lock);
	if (rc < 0)
		return (rc);
	return (np);
}

int	vfs_readdir(struct s_vfile *f, t_dirent *out)
{
	int	rc;

	if (f == NULL)
		return (E_BADF);
	if (out == NULL)
		return (E_INVAL);
	if (!(f->node->mode & S_TYPE_DIR))
		return (E_NOTDIR);
	memset(out, 0, sizeof(*out));
	mutex_lock(&f->lock);
	mutex_lock(&f->mnt->lock);
	rc = f->mnt->ops->readdir(f->mnt->fs, f->node, &f->pos, out);
	mutex_unlock(&f->mnt->lock);
	mutex_unlock(&f->lock);
	return (rc);
}
