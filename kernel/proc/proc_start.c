#include "proc_int.h"
#include "velum/err.h"
#include "velum/libk.h"

_Static_assert(PROC_HRIGHTS == (HR_WAIT | HR_SIGNAL | HR_READ | HR_DUP
		| HR_TRANSFER), "droits du handle de processus");
_Static_assert(THREAD_HRIGHTS == (HR_WAIT | HR_READ | HR_DUP | HR_TRANSFER),
	"droits du handle de fil");

void	proc_abort(t_process *p)
{
	t_object	*o;

	o = p->obj;
	p->obj = NULL;
	proc_remove(p);
	if (o)
		obj_unref(o);
	proc_free(p);
}

static int	start_handle(t_spawnctx *c)
{
	if (!c->hout)
		return (0);
	if (!c->p->obj)
		return (E_NOSYS);
	return (handle_alloc(c->rq->parent, c->p->obj, PROC_HRIGHTS, c->hout));
}

static t_thread	*start_thread(t_spawnctx *c)
{
	t_threadreq	rq;
	t_pthread	st;

	memset(&rq, 0, sizeof(rq));
	memset(&st, 0, sizeof(st));
	rq.name = c->p->name;
	rq.prio = PRIO_NORMAL;
	rq.flags = THREAD_USER;
	rq.proc = c->p;
	rq.entry = c->p->entry;
	rq.sp = c->sp;
	rq.arg = (void *)c->sp;
	return (proc_thread_add(c->p, &rq, &st));
}

static void	start_undo_handle(t_spawnctx *c)
{
	if (c->hout && *c->hout != HANDLE_INVALID && handle_close)
		handle_close(c->rq->parent, *c->hout);
	if (c->hout)
		*c->hout = HANDLE_INVALID;
}

int	proc_start_main(t_spawnctx *c)
{
	t_thread	*t;
	int			rc;

	rc = proc_make_obj(c->p);
	if (rc == 0)
		rc = proc_insert(c->p);
	if (rc == 0)
		rc = start_handle(c);
	if (rc < 0)
		return (rc);
	if (c->out)
	{
		proc_ref(c->p);
		*c->out = c->p;
	}
	t = start_thread(c);
	if (!t)
	{
		start_undo_handle(c);
		return (E_NOMEM);
	}
	thread_unref(t);
	return (0);
}
