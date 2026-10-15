#include "irq_int.h"
#include "velum/err.h"

static int	act_alloc(t_irq_core *c)
{
	int	k;

	k = 0;
	while (k < IRQ_ACTIONS_MAX)
	{
		if (!c->act[k].used)
			return (k);
		k++;
	}
	return (E_NOMEM);
}

int	irqc_attach(t_irq_core *c, int idx, t_irqfn fn, void *ctx)
{
	t_irq_slot	*s;
	int			a;
	int			k;

	s = &c->slot[idx];
	if (s->nact >= IRQ_SHARE_MAX)
		return (E_NOMEM);
	k = s->head;
	while (k != IRQ_NONE)
	{
		if (c->act[k].fn == fn)
			return (E_EXIST);
		k = c->act[k].next;
	}
	a = act_alloc(c);
	if (a < 0)
		return (a);
	c->act[a].fn = fn;
	c->act[a].ctx = ctx;
	c->act[a].used = 1;
	c->act[a].next = s->head;
	s->head = (int16_t)a;
	s->nact++;
	return (E_OK);
}

int	irqc_detach(t_irq_core *c, int idx, t_irqfn fn)
{
	int16_t	*link;
	int		k;

	link = &c->slot[idx].head;
	while (*link != IRQ_NONE)
	{
		k = *link;
		if (c->act[k].fn == fn)
		{
			*link = c->act[k].next;
			c->act[k].used = 0;
			c->act[k].fn = NULL;
			c->act[k].ctx = NULL;
			c->act[k].next = IRQ_NONE;
			c->slot[idx].nact--;
			return (c->slot[idx].nact);
		}
		link = &c->act[k].next;
	}
	return (E_NOENT);
}

static int	join_line(t_irq_core *c, int idx, const t_irq_req *rq)
{
	int	rc;

	if (!rq->shared || !c->slot[idx].shared || c->slot[idx].trig != rq->trig)
		return (E_BUSY);
	rc = irqc_attach(c, idx, rq->fn, rq->ctx);
	if (rc < 0)
		return (rc);
	return (idx);
}

int	irqc_request(t_irq_core *c, const t_irq_req *rq, bool *is_new)
{
	int	idx;
	int	rc;

	*is_new = false;
	idx = irqc_find_gsi(c, rq->gsi);
	if (idx >= 0)
		return (join_line(c, idx, rq));
	idx = irqc_vec_alloc(c);
	if (idx < 0)
		return (idx);
	c->slot[idx].gsi = rq->gsi;
	c->slot[idx].trig = rq->trig;
	c->slot[idx].shared = rq->shared;
	rc = irqc_attach(c, idx, rq->fn, rq->ctx);
	if (rc < 0)
	{
		irqc_vec_release(c, idx);
		return (rc);
	}
	*is_new = true;
	return (idx);
}
