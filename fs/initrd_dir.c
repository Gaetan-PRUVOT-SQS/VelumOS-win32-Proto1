#include "cpio.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	child_of(const t_cpent *e, const t_cspan *dir, t_cspan *c)
{
	const char	*r;
	uint32_t	rn;
	uint32_t	k;

	r = e->name;
	rn = e->nlen;
	if (dir->n)
	{
		if (e->nlen < dir->n || memcmp(e->name, dir->p, dir->n) != 0)
			return (-1);
		if (e->nlen == dir->n)
			return (0);
		if (e->name[dir->n] != '/')
			return (-1);
		r = e->name + dir->n + 1;
		rn = e->nlen - dir->n - 1;
	}
	k = 0;
	while (k < rn && r[k] != '/')
		k++;
	c->p = r;
	c->n = k;
	c->leaf = (k == rn);
	return (1);
}

static int	dup_prev(const t_initrd *rd, uint32_t i, const t_cspan *dir,
		const t_cspan *c)
{
	t_cspan	prev;

	if (i == 0 || child_of(&rd->ents[i - 1], dir, &prev) != 1)
		return (0);
	return (prev.n == c->n && memcmp(prev.p, c->p, c->n) == 0);
}

static int	try_emit(const t_initrd *rd, uint32_t i, const t_cspan *dir,
		t_dirent *o)
{
	t_cspan		c;
	uint64_t	h;
	int			r;

	r = child_of(&rd->ents[i], dir, &c);
	if (r < 0)
		return (-1);
	if (r == 0 || dup_prev(rd, i, dir, &c))
		return (0);
	h = cpio_hash(FNV_BASIS, dir->p, dir->n);
	if (dir->n)
		h = cpio_hash(h, "/", 1);
	o->inode = cpio_hash(h, c.p, c.n);
	o->type = S_TYPE_DIR;
	if (c.leaf && (rd->ents[i].mode & CPIO_TYPE_MASK) == CPIO_TYPE_REG)
		o->type = S_TYPE_REG;
	o->namelen = c.n;
	memcpy(o->name, c.p, c.n);
	o->name[c.n] = '\0';
	return (1);
}

static void	dir_span(const t_initrd *rd, const t_vnode *d, t_cspan *dir)
{
	dir->p = "";
	if (d->loc != VFS_NOLOC)
		dir->p = rd->ents[d->loc].name;
	dir->n = d->first;
	dir->leaf = 0;
}

int	rd_readdir(void *fs, t_vnode *d, uint64_t *ck, t_dirent *o)
{
	const t_initrd	*rd;
	t_cspan			dir;
	uint64_t		i;
	int				r;

	rd = fs;
	dir_span(rd, d, &dir);
	i = *ck - 1;
	if (*ck == 0)
		i = cpio_lower(rd, dir.p, dir.n);
	while (i < rd->n)
	{
		r = try_emit(rd, (uint32_t)i, &dir, o);
		if (r < 0)
			break ;
		if (r > 0)
		{
			*ck = i + 2;
			return (1);
		}
		i++;
	}
	*ck = (uint64_t)rd->n + 1;
	return (0);
}
