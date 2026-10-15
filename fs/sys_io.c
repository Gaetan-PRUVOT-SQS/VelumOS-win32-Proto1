#include "fsys.h"
#include "vfs_weak.h"
#include "velum/err.h"
#include "velum/heap.h"

static int64_t	io_chunk(t_fsio *io, uint8_t *kb, uint64_t done, uint64_t n)
{
	t_vio	v;
	int64_t	rc;

	v.dst = NULL;
	v.src = kb;
	if (!io->write)
	{
		v.dst = kb;
		v.src = NULL;
	}
	if (io->write && n && copy_from_user(kb, io->ubuf + done, n) < 0)
		return (E_FAULT);
	v.off = io->off + done;
	v.len = n;
	rc = vfs_xfer(io->f, &v, (int)io->write, (int)io->use_pos);
	if (rc > 0 && !io->write
		&& copy_to_user(io->ubuf + done, kb, (size_t)rc) < 0)
		return (E_FAULT);
	return (rc);
}

static int64_t	io_loop(t_fsio *io, uint8_t *kb, uint64_t cap)
{
	uint64_t	done;
	uint64_t	n;
	int64_t		rc;

	done = 0;
	rc = 0;
	while (done < io->len)
	{
		n = io->len - done;
		if (n > cap)
			n = cap;
		rc = io_chunk(io, kb, done, n);
		if (rc <= 0)
			break ;
		done += (uint64_t)rc;
		if ((uint64_t)rc < n)
			break ;
	}
	if (done)
		return ((int64_t)done);
	return (rc);
}

static int64_t	fsys_io(t_fsio *io)
{
	uint8_t		*kb;
	uint64_t	cap;
	int64_t		rc;

	if (io->len > VFS_IO_MAX)
		io->len = VFS_IO_MAX;
	if (io->len == 0)
		return (io_chunk(io, NULL, 0, 0));
	if (!user_range_ok(io->ubuf, io->len))
		return (E_FAULT);
	cap = io->len;
	if (cap > FSYS_CHUNK)
		cap = FSYS_CHUNK;
	kb = kmalloc_tag(cap, HEAP_FS);
	if (kb == NULL)
		return (E_NOMEM);
	rc = io_loop(io, kb, cap);
	kfree(kb);
	return (rc);
}

int64_t	fsys_rw(const t_sysargs *a, uint32_t write, uint32_t use_pos)
{
	t_hget		hg;
	t_fsio		io;
	int64_t		rc;
	uint32_t	right;
	int			r;

	right = HR_READ;
	if (write)
		right = HR_WRITE;
	r = fsys_get(a->a[0], OBJ_FILE, right, &hg);
	if (r < 0)
		return (r);
	io.f = hg.obj->impl;
	io.ubuf = a->a[1];
	io.len = a->a[2];
	io.off = a->a[3];
	io.write = write;
	io.use_pos = use_pos;
	rc = fsys_io(&io);
	obj_unref(hg.obj);
	return (rc);
}
