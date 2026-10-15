#include "obj_int.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/util.h"

int	secmap_insert(t_htab *ht, t_secmap *m)
{
	uint64_t	fl;
	int			rc;

	fl = spin_lock_irqsave(&ht->lock);
	rc = E_NOMEM;
	if (ht->closed)
		rc = E_CANCELED;
	else if (ht->nmaps < SEC_MAPS_MAX)
	{
		m->next = ht->maps;
		ht->maps = m;
		ht->nmaps++;
		obj_ref(m->sec);
		rc = 0;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
	return (rc);
}

static t_secmap	*secmaps_detach(t_htab *ht)
{
	uint64_t	fl;
	t_secmap	*m;

	fl = spin_lock_irqsave(&ht->lock);
	m = ht->maps;
	ht->maps = NULL;
	ht->nmaps = 0;
	spin_unlock_irqrestore(&ht->lock, fl);
	return (m);
}

void	secmap_drop(t_aspace *as, t_secmap *m)
{
	if (as && sec_pages_unmap(as, m) > 0)
		klog_warn("objets: section restee mappee, reference gardee");
	else
		obj_unref(m->sec);
	kfree(m);
}

void	secmaps_cleanup(t_process *p)
{
	t_secmap	*m;
	t_secmap	*next;

	if (!ht_of(p))
		return ;
	m = secmaps_detach(ht_of(p));
	while (m)
	{
		next = m->next;
		if (p->aspace)
			sec_pages_unmap(p->aspace, m);
		secmap_drop(NULL, m);
		m = next;
	}
}

void	secmaps_abandon(t_htab *ht)
{
	t_secmap	*m;
	t_secmap	*next;

	m = secmaps_detach(ht);
	while (m)
	{
		next = m->next;
		kfree(m);
		m = next;
	}
}
