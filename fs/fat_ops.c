#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

int	fat_parent(t_fat *fs, const char *rel, t_vnode *pn, const char **name)
{
	char	parent[VFS_PATH_MAX];
	int		rc;

	rc = vpath_split(rel, parent, name);
	if (rc < 0)
		return (rc);
	rc = fat_op_lookup(fs, parent, pn);
	if (rc < 0)
		return (rc);
	if (!(pn->mode & S_TYPE_DIR))
		return (E_NOTDIR);
	return (0);
}

int	fat_op_lookup(void *fs, const char *rel, t_vnode *out)
{
	char		comp[VFS_NAME_MAX + 1];
	t_fent		ent;
	const char	*p;
	int			rc;

	fat_root_node(fs, out);
	p = vpath_next(rel, comp);
	while (p)
	{
		if (!(out->mode & S_TYPE_DIR))
			return (E_NOTDIR);
		rc = fdir_find(fs, out->first, comp, &ent);
		if (rc == 0)
			rc = fat_node_from(fs, &ent, out);
		if (rc < 0)
			return (rc);
		p = vpath_next(p, comp);
	}
	return (0);
}

int	fat_op_readdir(void *fs, t_vnode *d, uint64_t *ck, t_dirent *o)
{
	t_fdirit	it;
	t_fent		ent;
	int			rc;

	if (*ck >= FAT_DIR_MAX)
		return (0);
	fdir_init(&it, d->first);
	it.idx = (uint32_t)(*ck);
	rc = fdir_scan(fs, &it, &ent);
	*ck = it.idx;
	if (rc == 0)
		*ck = FAT_DIR_MAX;
	if (rc <= 0)
		return (rc);
	o->inode = ent.pos;
	o->type = S_TYPE_REG;
	if (ent.raw[11] & FAT_A_DIR)
		o->type = S_TYPE_DIR;
	o->namelen = (uint32_t)strlcpy(o->name, ent.name, sizeof(o->name));
	return (1);
}

static int	create_raw(t_fat *fs, uint32_t parent, uint32_t t, t_fent *ent)
{
	uint32_t	c;
	int			rc;

	if (t != S_TYPE_DIR)
	{
		fat_tmpl(ent->raw, FAT_A_ARC, 0);
		return (0);
	}
	fat_tmpl(ent->raw, FAT_A_DIR, 0);
	rc = fat_mkdir_clus(fs, parent, ent->raw, &c);
	if (rc < 0)
		return (rc);
	fat_tmpl(ent->raw, FAT_A_DIR, c);
	return (0);
}

int	fat_op_create(void *fs, const char *rel, uint32_t t, t_vnode *o)
{
	t_vnode		pn;
	t_fent		ent;
	const char	*name;
	uint32_t	c;
	int			rc;

	if (((t_fat *)fs)->ro)
		return (E_PERM);
	rc = fat_parent(fs, rel, &pn, &name);
	if (rc == 0)
		rc = fat_name_valid(name);
	if (rc == 0)
		rc = create_raw(fs, pn.first, t, &ent);
	if (rc < 0)
		return (fat_commit(fs, rc));
	c = (le16(ent.raw + 20) << 16) | le16(ent.raw + 26);
	rc = fdir_add(fs, pn.first, name, &ent);
	if (rc < 0 && c)
		fat_free_chain(fs, c);
	rc = fat_commit(fs, rc);
	if (rc < 0)
		return (rc);
	return (fat_node_from(fs, &ent, o));
}
