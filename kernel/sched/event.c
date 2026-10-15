#include "sched_int.h"

void	event_init(t_event *ev, bool manual, bool initial)
{
	spin_init(&ev->lock, "event");
	waitq_init(&ev->wq);
	ev->signaled = initial;
	ev->manual = manual;
}

void	event_set(t_event *ev)
{
	uint64_t	flags;

	flags = spin_lock_irqsave(&ev->lock);
	if (ev->manual)
	{
		ev->signaled = true;
		waitq_wake_all(&ev->wq);
	}
	else if (!wq_wake_one(&ev->wq, 0))
		ev->signaled = true;
	spin_unlock_irqrestore(&ev->lock, flags);
}

void	event_reset(t_event *ev)
{
	uint64_t	flags;

	flags = spin_lock_irqsave(&ev->lock);
	ev->signaled = false;
	spin_unlock_irqrestore(&ev->lock, flags);
}

int	event_wait(t_event *ev, uint64_t timeout_ns)
{
	uint64_t	flags;
	int			rc;

	flags = spin_lock_irqsave(&ev->lock);
	if (ev->signaled)
	{
		if (!ev->manual)
			ev->signaled = false;
		spin_unlock_irqrestore(&ev->lock, flags);
		return (0);
	}
	rc = waitq_wait(&ev->wq, &ev->lock, timeout_ns);
	spin_unlock_irqrestore(&ev->lock, flags);
	return (rc);
}
