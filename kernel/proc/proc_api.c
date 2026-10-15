#include "proc_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static t_spawnctx	*spawn_ctx_new(const t_spawnreq *rq)
{
	t_spawnctx	*c;

	c = kmalloc_tag(sizeof(*c), HEAP_PROC);
	if (!c)
		return (NULL);
	memset(c, 0, sizeof(*c));
	c->rq = rq;
	return (c);
}

int	proc_spawn(const t_spawnreq *rq, t_process **out)
{
	t_spawnctx	*c;
	int			rc;

	if (!rq)
		return (E_INVAL);
	if (!proc_tab()->ready)
		return (E_NOSYS);
	c = spawn_ctx_new(rq);
	if (!c)
		return (E_NOMEM);
	c->out = out;
	rc = spawn_run(c);
	kfree(c);
	return (rc);
}

int	proc_spawn_handle(const t_spawnreq *rq, t_handle *h)
{
	t_spawnctx	*c;
	int			rc;

	if (!rq || !rq->parent || !h)
		return (E_INVAL);
	if (!proc_tab()->ready || !handle_alloc || !obj_process_new)
		return (E_NOSYS);
	c = spawn_ctx_new(rq);
	if (!c)
		return (E_NOMEM);
	c->hout = h;
	rc = spawn_run(c);
	kfree(c);
	return (rc);
}
