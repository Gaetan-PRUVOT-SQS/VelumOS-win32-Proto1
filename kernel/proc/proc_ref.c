#include "proc_int.h"
#include "velum/panic.h"

t_procext	*proc_ext(t_process *p)
{
	return (&((t_procbox *)p)->ext);
}

static int	proc_tryref(t_process *p)
{
	uint32_t	n;

	n = __atomic_load_n(&p->refs, __ATOMIC_ACQUIRE);
	while (n != 0)
	{
		if (__atomic_compare_exchange_n(&p->refs, &n, n + 1, 0,
				__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
			return (1);
	}
	return (0);
}

t_process	*proc_find(uint32_t pid)
{
	t_proctab	*tab;
	t_process	*found;
	uint64_t	fl;
	uint32_t	i;

	tab = proc_tab();
	found = NULL;
	fl = spin_lock_irqsave(&tab->lock);
	i = 0;
	while (i < PROC_MAX && !found)
	{
		if (tab->slot[i] && tab->slot[i]->pid == pid
			&& proc_tryref(tab->slot[i]))
			found = tab->slot[i];
		i++;
	}
	spin_unlock_irqrestore(&tab->lock, fl);
	return (found);
}

void	proc_ref(t_process *p)
{
	uint32_t	n;

	n = __atomic_add_fetch(&p->refs, 1, __ATOMIC_ACQ_REL);
	kassert_check(n > 1, "proc: référence prise sur un processus libéré");
}

void	proc_unref(t_process *p)
{
	uint32_t	n;

	if (!p)
		return ;
	n = __atomic_sub_fetch(&p->refs, 1, __ATOMIC_ACQ_REL);
	kassert_check(n != UINT32_MAX, "proc: compteur de références négatif");
	if (n != 0)
		return ;
	proc_remove(p);
	proc_free(p);
}
