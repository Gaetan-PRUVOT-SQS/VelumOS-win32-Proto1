#include "sched_int.h"
#include "velum/panic.h"

void	mutex_init(t_mutex *m, const char *name)
{
	spin_init(&m->lock, "mutex");
	m->owner = NULL;
	waitq_init(&m->wq);
	m->name = name;
	if (!name)
		m->name = "mutex";
}

static void	mutex_acquire(t_mutex *m, t_kthread *cur)
{
	int32_t	top;

	while (m->owner != NULL)
	{
		if (m->owner == &cur->t)
			panic("mutex '%s' : verrouillage récursif par le fil %u",
				m->name, cur->t.tid);
		sched_prio_inherit((t_kthread *)m->owner, cur->t.prio);
		wq_block(&m->wq, &m->lock, TIMEOUT_NONE, false);
	}
	m->owner = &cur->t;
	if (!m->wq.head)
		return ;
	top = waitq_list_max_prio(&m->wq);
	if (top >= 0)
		sched_prio_inherit(cur, top);
}

void	mutex_lock(t_mutex *m)
{
	t_kthread	*cur;
	uint64_t	flags;

	cur = sched_kself();
	flags = spin_lock_irqsave(&m->lock);
	mutex_acquire(m, cur);
	spin_unlock_irqrestore(&m->lock, flags);
}

bool	mutex_trylock(t_mutex *m)
{
	uint64_t	flags;
	bool		ok;

	flags = spin_lock_irqsave(&m->lock);
	ok = (m->owner == NULL);
	if (ok)
		m->owner = &sched_kself()->t;
	spin_unlock_irqrestore(&m->lock, flags);
	return (ok);
}

void	mutex_unlock(t_mutex *m)
{
	t_kthread	*cur;
	uint64_t	flags;

	cur = sched_kself();
	flags = spin_lock_irqsave(&m->lock);
	if (m->owner != &cur->t)
		panic("mutex '%s' : déverrouillé par le fil %u qui ne le tient pas",
			m->name, cur->t.tid);
	m->owner = NULL;
	sched_prio_uninherit(cur);
	wq_wake_one(&m->wq, 0);
	spin_unlock_irqrestore(&m->lock, flags);
}
