#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"

static int	wb_block(t_wblock *wb, uint64_t deadline)
{
	uint64_t	now;

	if (deadline == TIMEOUT_INF)
		return (waitq_wait(&wb->wq, &wb->lock, TIMEOUT_NONE));
	now = time_now_ns();
	if (now >= deadline)
		return (E_TIMEOUT);
	return (waitq_wait(&wb->wq, &wb->lock, deadline - now));
}

static int	wset_sleep(t_waitset *ws, uint64_t deadline)
{
	uint64_t	fl;
	int			rc;

	fl = spin_lock_irqsave(&ws->wb.lock);
	rc = 0;
	if (!ws->wb.fired)
		rc = wb_block(&ws->wb, deadline);
	ws->wb.fired = 0;
	spin_unlock_irqrestore(&ws->wb.lock, fl);
	return (rc);
}

static int	wset_loop(t_waitset *ws, uint64_t deadline)
{
	int	rc;

	while (1)
	{
		rc = wset_try(ws);
		if (rc >= 0)
			return (rc);
		rc = wset_sleep(ws, deadline);
		if (rc == E_TIMEOUT)
		{
			rc = wset_try(ws);
			if (rc >= 0)
				return (rc);
			return (E_TIMEOUT);
		}
		if (rc < 0)
			return (rc);
	}
}

static void	wset_init(t_waitset *ws, t_object **objs, uint32_t n,
		uint32_t mode)
{
	uint32_t	i;

	memset(ws, 0, sizeof(t_waitset));
	spin_init(&ws->wb.lock, "wait-block");
	waitq_init(&ws->wb.wq);
	ws->n = n;
	ws->mode = mode;
	i = 0;
	while (i < n)
	{
		ws->objs[i] = objs[i];
		ws->links[i].obj = objs[i];
		ws->links[i].wb = &ws->wb;
		i++;
	}
}

int	wait_objects(t_object **o, uint32_t n, uint32_t m, uint64_t to)
{
	t_waitset	*ws;
	uint64_t	deadline;
	int			rc;

	if (!o || n == 0 || n > WAIT_MAX || m > WAIT_ALL)
		return (E_INVAL);
	deadline = wait_deadline(to);
	ws = kmalloc_tag(sizeof(t_waitset), HEAP_OBJECT);
	if (!ws)
		return (E_NOMEM);
	wset_init(ws, o, n, m);
	wreg_add(ws);
	rc = wset_loop(ws, deadline);
	wreg_del(ws);
	kfree(ws);
	return (rc);
}
