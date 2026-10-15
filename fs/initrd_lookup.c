#include "cpio.h"
#include "velum/err.h"
#include "velum/libk.h"

static void	node_dir(t_vnode *o, uint64_t ino, uint64_t loc, uint32_t nlen)
{
	memset(o, 0, sizeof(*o));
	o->ino = ino;
	o->mode = S_TYPE_DIR | 0555;
	o->attr = VFS_STAT_RO;
	o->loc = loc;
	o->first = nlen;
}

static void	node_ent(const t_initrd *rd, uint32_t i, uint64_t ino, t_vnode *o)
{
	const t_cpent	*e;

	e = &rd->ents[i];
	node_dir(o, ino, i, e->nlen);
	if ((e->mode & CPIO_TYPE_MASK) == CPIO_TYPE_REG)
	{
		o->mode = S_TYPE_REG | 0444;
		o->size = e->size;
	}
	o->mtime_ns = e->mtime * 1000000000ull;
	o->ctime_ns = o->mtime_ns;
}

static int	lookup_entry(const t_initrd *rd, const char *rel, uint32_t klen,
		t_vnode *out)
{
	const t_cpent	*e;
	uint32_t		i;
	uint64_t		ino;

	i = cpio_lower(rd, rel, klen);
	if (i >= rd->n)
		return (E_NOENT);
	e = &rd->ents[i];
	ino = cpio_hash(FNV_BASIS, rel, klen);
	if (cpio_cmp(e->name, e->nlen, rel, klen) == 0)
	{
		node_ent(rd, i, ino, out);
		return (0);
	}
	if (e->nlen > klen && memcmp(e->name, rel, klen) == 0
		&& e->name[klen] == '/')
	{
		node_dir(out, ino, i, klen);
		return (0);
	}
	return (E_NOENT);
}

int	rd_lookup(void *fs, const char *rel, t_vnode *out)
{
	uint32_t	klen;

	klen = (uint32_t)strlen(rel);
	if (klen == 0)
	{
		node_dir(out, FNV_BASIS, VFS_NOLOC, 0);
		return (0);
	}
	return (lookup_entry(fs, rel, klen, out));
}

int64_t	rd_read(void *fs, t_vnode *n, t_vio *io)
{
	const t_cpent	*e;
	uint64_t		len;

	if (n->loc == VFS_NOLOC || (n->mode & S_TYPE_DIR))
		return (E_ISDIR);
	e = &((const t_initrd *)fs)->ents[n->loc];
	if (io->off >= e->size)
		return (0);
	len = e->size - io->off;
	if (io->len < len)
		len = io->len;
	memcpy(io->dst, e->data + io->off, len);
	return ((int64_t)len);
}
