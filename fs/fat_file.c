#include "fat.h"
#include "velum/err.h"

int64_t	fat_op_read(void *fs, t_vnode *n, t_vio *io)
{
	t_vio	sub;

	if (n->mode & S_TYPE_DIR)
		return (E_ISDIR);
	if (io->off >= n->size)
		return (0);
	sub = *io;
	if (sub.len > n->size - io->off)
		sub.len = n->size - io->off;
	return (fat_xfer(fs, n, &sub, 0));
}

int	fat_fill_zero(t_fat *fs, t_vnode *n, uint64_t to)
{
	t_vio	z;
	int64_t	rc;

	z.dst = NULL;
	z.src = NULL;
	z.off = n->size;
	z.len = to - n->size;
	rc = fat_xfer(fs, n, &z, 1);
	if (rc > 0)
		n->size += (uint64_t)rc;
	if (rc < 0)
		return ((int)rc);
	if ((uint64_t)rc < z.len)
		return (fs->xerr);
	return (0);
}

static int64_t	write_body(t_fat *fs, t_vnode *n, const t_vio *io, uint64_t len)
{
	t_vio	sub;
	int64_t	rc;

	sub = *io;
	sub.len = len;
	rc = fat_xfer(fs, n, &sub, 1);
	if (rc > 0 && io->off + (uint64_t)rc > n->size)
		n->size = io->off + (uint64_t)rc;
	return (rc);
}

int64_t	fat_op_write(void *fs, t_vnode *n, t_vio *io)
{
	t_fat		*f;
	uint64_t	len;
	int64_t		rc;
	int			rs;

	f = fs;
	if (f->ro)
		return (E_PERM);
	if (n->mode & S_TYPE_DIR)
		return (E_ISDIR);
	if (io->off >= FAT_FILE_MAX)
		return (E_RANGE);
	len = io->len;
	if (len > FAT_FILE_MAX - io->off)
		len = FAT_FILE_MAX - io->off;
	rc = 0;
	if (io->off > n->size)
		rc = fat_fill_zero(f, n, io->off);
	if (rc == 0)
		rc = write_body(f, n, io, len);
	rs = fat_commit(f, fat_node_sync(f, n, 1));
	if (rc >= 0 && rs < 0)
		return (rs);
	return (rc);
}
