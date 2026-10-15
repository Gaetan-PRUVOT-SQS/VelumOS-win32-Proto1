#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

void	fat_root_node(const t_fat *fs, t_vnode *o)
{
	memset(o, 0, sizeof(*o));
	o->ino = FAT_ROOT_INO;
	o->loc = 0;
	o->first = fs->root;
	o->mode = S_TYPE_DIR | 0755;
}

int	fat_node_from(const t_fat *fs, const t_fent *e, t_vnode *o)
{
	const uint8_t	*r;

	r = e->raw;
	memset(o, 0, sizeof(*o));
	o->first = (le16(r + 20) << 16) | le16(r + 26);
	o->attr = r[11];
	o->ino = e->pos;
	o->loc = e->pos;
	o->mtime_ns = fat_time_unpack(le16(r + 24), le16(r + 22));
	o->ctime_ns = fat_time_unpack(le16(r + 16), le16(r + 14));
	if (r[11] & FAT_A_DIR)
	{
		o->mode = S_TYPE_DIR | 0755;
		if (!fat_ok(fs, o->first))
			return (E_IO);
		return (0);
	}
	o->mode = S_TYPE_REG | 0644;
	if (r[11] & FAT_A_RO)
		o->mode = S_TYPE_REG | 0444;
	o->size = le32(r + 28);
	if (o->first && !fat_ok(fs, o->first))
		return (E_IO);
	return (0);
}

int	fat_node_sync(t_fat *fs, t_vnode *n, int touch)
{
	uint8_t		e[FAT_DENT];
	uint16_t	d;
	uint16_t	t;
	int			rc;

	if (n->loc == 0)
		return (0);
	rc = fdir_get(fs, n->loc, e);
	if (rc < 0)
		return (rc);
	put16(e + 20, n->first >> 16);
	put16(e + 26, n->first & 0xFFFF);
	if (!(n->mode & S_TYPE_DIR))
		put32(e + 28, (uint32_t)n->size);
	if (touch)
	{
		fat_time_pack(vfs_now_ns(), &d, &t);
		put16(e + 18, d);
		put16(e + 22, t);
		put16(e + 24, d);
		e[11] |= FAT_A_ARC;
		n->mtime_ns = fat_time_unpack(d, t);
	}
	return (fdir_put(fs, n->loc, e));
}

void	fat_tmpl(uint8_t *raw, uint32_t attr, uint32_t first)
{
	uint16_t	d;
	uint16_t	t;

	memset(raw, 0, FAT_DENT);
	raw[11] = (uint8_t)attr;
	fat_time_pack(vfs_now_ns(), &d, &t);
	put16(raw + 14, t);
	put16(raw + 16, d);
	put16(raw + 18, d);
	put16(raw + 22, t);
	put16(raw + 24, d);
	put16(raw + 20, first >> 16);
	put16(raw + 26, first & 0xFFFF);
}

int	fat_op_open(void *fs, t_vnode *n)
{
	t_fat		*f;
	uint64_t	need;

	f = fs;
	need = 0;
	if (!(n->mode & S_TYPE_DIR))
		need = (n->size + f->clsz - 1) / f->clsz;
	return (fat_chain_check(f, n->first, need));
}
