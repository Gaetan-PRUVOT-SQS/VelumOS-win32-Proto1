#include "sched_int.h"

void	sem_init(t_sem *s, int64_t count)
{
	spin_init(&s->lock, "sem");
	waitq_init(&s->wq);
	s->count = count;
	if (count < 0)
		s->count = 0;
}

int	sem_wait(t_sem *s, uint64_t timeout_ns)
{
	uint64_t	flags;
	int			rc;

	flags = spin_lock_irqsave(&s->lock);
	if (s->count > 0)
	{
		s->count--;
		spin_unlock_irqrestore(&s->lock, flags);
		return (0);
	}
	rc = waitq_wait(&s->wq, &s->lock, timeout_ns);
	spin_unlock_irqrestore(&s->lock, flags);
	return (rc);
}

void	sem_post(t_sem *s)
{
	uint64_t	flags;

	flags = spin_lock_irqsave(&s->lock);
	if (!wq_wake_one(&s->wq, 0) && s->count < INT64_MAX)
		s->count++;
	spin_unlock_irqrestore(&s->lock, flags);
}
