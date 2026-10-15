#include "vfs_int.h"
#include "velum/err.h"

static int	xfer_check(t_vfile *f, int write)
{
	uint32_t	acc;

	acc = f->flags & VFS_ACC_MASK;
	if (f->node->mode & S_TYPE_DIR)
		return (E_ISDIR);
	if (write && acc == O_RDONLY)
		return (E_BADF);
	if (!write && acc == O_WRONLY)
		return (E_BADF);
	if (write && (f->mnt->flags & VFS_MOUNT_RO))
		return (E_PERM);
	return (0);
}

static int64_t	xfer_locked(t_vfile *f, t_vio *io, int write)
{
	if (write && (f->flags & O_APPEND))
		io->off = f->node->size;
	if (io->off > VFS_OFF_MAX)
		return (E_INVAL);
	if (io->len == 0)
		return (0);
	if (write)
		return (f->mnt->ops->write(f->mnt->fs, f->node, io));
	return (f->mnt->ops->read(f->mnt->fs, f->node, io));
}

int64_t	vfs_xfer(t_vfile *f, t_vio *io, int write, int use_pos)
{
	int64_t	rc;

	if (f == NULL)
		return (E_BADF);
	rc = xfer_check(f, write);
	if (rc < 0)
		return (rc);
	if (io->len > VFS_IO_MAX)
		io->len = VFS_IO_MAX;
	if (use_pos)
	{
		mutex_lock(&f->lock);
		io->off = f->pos;
	}
	mutex_lock(&f->mnt->lock);
	rc = xfer_locked(f, io, write);
	mutex_unlock(&f->mnt->lock);
	if (use_pos)
	{
		if (rc > 0)
			f->pos = io->off + (uint64_t)rc;
		mutex_unlock(&f->lock);
	}
	return (rc);
}

int64_t	vfs_read(struct s_vfile *f, void *buf, size_t n)
{
	t_vio	io;

	if (buf == NULL && n)
		return (E_INVAL);
	io.dst = buf;
	io.src = NULL;
	io.len = n;
	io.off = 0;
	return (vfs_xfer(f, &io, 0, 1));
}

int64_t	vfs_write(struct s_vfile *f, const void *buf, size_t n)
{
	t_vio	io;

	if (buf == NULL && n)
		return (E_INVAL);
	io.dst = NULL;
	io.src = buf;
	io.len = n;
	io.off = 0;
	return (vfs_xfer(f, &io, 1, 1));
}
