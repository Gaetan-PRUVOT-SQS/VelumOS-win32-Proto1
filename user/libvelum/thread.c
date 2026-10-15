#include "stdlib.h"
#include "thread_int.h"
#include "velum/err.h"
#include "velum/vproc.h"

static int	thread_request(const t_vthreadattr *attr, t_vthreadreq *rq)
{
	rq->stack_size = V_THREAD_STACK_DEFAULT;
	rq->prio = V_PRIO_NORMAL;
	rq->flags = 0;
	if (!attr)
		return (0);
	if (attr->stack_size)
		rq->stack_size = attr->stack_size;
	if (rq->stack_size < V_THREAD_STACK_MIN
		|| rq->stack_size > V_THREAD_STACK_MAX
		|| attr->prio > V_PRIO_MAX)
		return (E_INVAL);
	if (attr->prio)
		rq->prio = attr->prio;
	rq->flags = attr->flags;
	return (0);
}

static int	thread_make(t_vthread **out, t_vthreadfn fn, void *arg)
{
	t_vthread	*t;
	int			rc;

	t = calloc(1, sizeof(*t));
	if (!t)
		return (E_NOMEM);
	rc = v_event_init(&t->done, 1, 0);
	if (rc < 0)
	{
		free(t);
		return (rc);
	}
	t->tcb.self = &t->tcb;
	t->tcb.thread = t;
	t->fn = fn;
	t->arg = arg;
	*out = t;
	return (0);
}

int	v_thread_create_ex(t_vthread **out, t_vthreadfn fn, void *arg,
		const t_vthreadattr *attr)
{
	t_vthreadreq	rq;
	t_vthread		*t;
	int64_t			h;
	int				rc;

	if (!out || !fn)
		return (E_INVAL);
	rc = thread_request(attr, &rq);
	if (rc < 0)
		return (rc);
	rc = thread_make(&t, fn, arg);
	if (rc < 0)
		return (rc);
	rq.entry = (uint64_t)(uintptr_t)__velum_thread_entry;
	rq.arg = (uint64_t)(uintptr_t)t;
	h = v_thread_create_raw(&rq);
	if (h < 0)
	{
		thread_cleanup(t);
		return ((int)h);
	}
	t->handle = (uint32_t)h;
	*out = t;
	return (0);
}

int	v_thread_create(t_vthread **out, t_vthreadfn fn, void *arg)
{
	return (v_thread_create_ex(out, fn, arg, NULL));
}
