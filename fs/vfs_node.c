#include "vfs_int.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/libk.h"

static t_vnode	*vnode_find(t_vmount *m, uint64_t ino)
{
	t_vnode	*n;

	n = m->open;
	while (n && n->ino != ino)
		n = n->next;
	return (n);
}

static int	vnode_new(t_vmount *m, const t_vnode *tmpl, t_vnode **out)
{
	t_vnode	*n;
	int		rc;

	n = vfs_alloc(sizeof(*n));
	if (n == NULL)
		return (E_NOMEM);
	*n = *tmpl;
	n->refs = 1;
	n->cur_idx = 0;
	n->cur_clus = 0;
	rc = m->ops->open(m->fs, n);
	if (rc < 0)
	{
		kfree(n);
		return (rc);
	}
	n->next = m->open;
	m->open = n;
	*out = n;
	return (0);
}

int	vnode_get(t_vmount *m, const t_vnode *tmpl, t_vnode **out)
{
	t_vnode	*n;

	n = vnode_find(m, tmpl->ino);
	if (n == NULL)
		return (vnode_new(m, tmpl, out));
	n->refs++;
	*out = n;
	return (0);
}

void	vnode_put(t_vmount *m, t_vnode *n)
{
	t_vnode	**link;

	if (--n->refs)
		return ;
	link = &m->open;
	while (*link && *link != n)
		link = &(*link)->next;
	if (*link)
		*link = n->next;
	kfree(n);
}

int	vnode_busy(t_vmount *m, uint64_t ino)
{
	return (vnode_find(m, ino) != NULL);
}
