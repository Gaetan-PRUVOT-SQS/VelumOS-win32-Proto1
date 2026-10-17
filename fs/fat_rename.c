#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

int	fat_op_remove(void *fs, const char *rel)
{
	t_vnode		pn;
	t_vnode		n;
	t_fent		ent;
	const char	*name;
	int			rc;

	if (((t_fat *)fs)->ro)
		return (E_PERM);
	rc = fat_parent(fs, rel, &pn, &name);
	if (rc == 0)
		rc = fdir_find(fs, pn.first, name, &ent);
	if (rc == 0)
		rc = fat_node_from(fs, &ent, &n);
	if (rc < 0)
		return (rc);
	if (n.attr & FAT_A_RO)
		return (E_ACCES);
	if (n.mode & S_TYPE_DIR)
		rc = fdir_empty(fs, n.first);
	if (rc == 0)
		rc = fdir_del(fs, pn.first, &ent);
	if (rc == 0 && n.first)
		rc = fat_free_chain(fs, n.first);
	return (fat_commit(fs, rc));
}

static int	subtree_ok(t_fat *fs, uint32_t src, uint32_t dest)
{
	uint32_t	c;
	uint32_t	depth;
	int			rc;

	c = dest;
	depth = 0;
	while (c != fs->root)
	{
		if (c == src)
			return (E_INVAL);
		if (++depth > FAT_DEPTH_MAX)
			return (E_IO);
		rc = fat_parent_of(fs, c, &c);
		if (rc < 0)
			return (rc);
	}
	return (0);
}

static int	rename_check(t_fat *fs, t_frename *r)
{
	int	rc;

	r->isdir = (r->src.raw[11] & FAT_A_DIR) != 0;
	r->sfirst = (le16(r->src.raw + 20) << 16) | le16(r->src.raw + 26);
	if (r->isdir)
	{
		rc = subtree_ok(fs, r->sfirst, r->pt.first);
		if (rc < 0)
			return (rc);
	}
	rc = fdir_find(fs, r->pt.first, r->nt, &r->dst);
	r->same = (rc == 0 && r->dst.pos == r->src.pos);
	if (rc == 0 && !r->same)
		return (E_EXIST);
	if (rc < 0 && rc != E_NOENT)
		return (rc);
	return (0);
}

static int	rename_same(t_fat *fs, t_frename *r)
{
	int	rc;

	rc = fdir_del(fs, r->pf.first, &r->src);
	if (rc < 0)
		return (rc);
	rc = fdir_add(fs, r->pt.first, r->nt, &r->dst);
	if (rc < 0)
		fdir_add(fs, r->pf.first, r->src.name, &r->src);
	return (rc);
}

static int	rename_move(t_fat *fs, t_frename *r)
{
	bool	dotdot;
	int		rc;

	dotdot = r->isdir && r->pf.first != r->pt.first;
	rc = fdir_add(fs, r->pt.first, r->nt, &r->dst);
	if (rc < 0)
		return (rc);
	if (dotdot)
		rc = fat_set_dotdot(fs, r->sfirst, r->pt.first);
	if (rc == 0)
		rc = fdir_del(fs, r->pf.first, &r->src);
	if (rc < 0)
	{
		fdir_del(fs, r->pt.first, &r->dst);
		if (dotdot)
			fat_set_dotdot(fs, r->sfirst, r->pf.first);
	}
	return (rc);
}

static int	rename_apply(t_fat *fs, t_frename *r)
{
	memcpy(r->dst.raw, r->src.raw, FAT_DENT);
	if (r->same)
		return (rename_same(fs, r));
	return (rename_move(fs, r));
}

int	fat_op_rename(void *fs, const char *from, const char *to)
{
	t_frename	r;
	int			rc;

	if (((t_fat *)fs)->ro)
		return (E_PERM);
	rc = fat_parent(fs, from, &r.pf, &r.nf);
	if (rc == 0)
		rc = fat_parent(fs, to, &r.pt, &r.nt);
	if (rc == 0)
		rc = fdir_find(fs, r.pf.first, r.nf, &r.src);
	if (rc == 0)
		rc = fat_name_valid(r.nt);
	if (rc == 0)
		rc = rename_check(fs, &r);
	if (rc == 0)
		rc = rename_apply(fs, &r);
	return (fat_commit(fs, rc));
}
