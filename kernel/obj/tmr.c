#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"

t_tmrtab				g_tmr;
static const t_objops	g_tmr_ops = {"timer", tmr_destroy, sig_signaled};

void	tmr_init(void)
{
	memset(&g_tmr, 0, sizeof(t_tmrtab));
	spin_init(&g_tmr.lock, "tmr");
}

void	tmr_destroy(t_object *o)
{
	t_tmrimpl	*t;
	t_tmrslot	*s;
	uint64_t	fl;
	int64_t		tid;

	t = o->impl;
	tid = 0;
	if (t->slot < TMR_SLOTS)
	{
		fl = spin_lock_irqsave(&g_tmr.lock);
		s = &g_tmr.slots[t->slot];
		tid = s->tid;
		s->obj = NULL;
		s->tid = 0;
		s->armgen++;
		spin_unlock_irqrestore(&g_tmr.lock, fl);
	}
	if (tid > 0)
		timer_cancel(tid);
	kfree(t);
}

static int	tmr_slot_alloc(t_object *o)
{
	uint64_t	fl;
	uint32_t	i;

	fl = spin_lock_irqsave(&g_tmr.lock);
	i = 0;
	while (i < TMR_SLOTS && g_tmr.slots[i].obj)
		i++;
	if (i < TMR_SLOTS)
	{
		g_tmr.slots[i].obj = o;
		g_tmr.slots[i].armgen++;
		g_tmr.slots[i].tid = 0;
		g_tmr.slots[i].period = 0;
	}
	spin_unlock_irqrestore(&g_tmr.lock, fl);
	if (i == TMR_SLOTS)
		return (E_NOMEM);
	return ((int)i);
}

int	tmr_create(uint32_t manual, t_object **out)
{
	t_tmrimpl	*t;
	int			slot;

	if (manual > 1 || !out)
		return (E_INVAL);
	t = kmalloc_tag(sizeof(t_tmrimpl), HEAP_OBJECT);
	if (!t)
		return (E_NOMEM);
	t->sig.state = 0;
	t->sig.manual = manual;
	t->slot = TMR_SLOTS;
	*out = obj_create(OBJ_TIMER, &g_tmr_ops, t);
	if (!*out)
	{
		kfree(t);
		return (E_NOMEM);
	}
	slot = tmr_slot_alloc(*out);
	if (slot >= 0)
		t->slot = (uint32_t)slot;
	if (slot >= 0)
		return (0);
	obj_unref(*out);
	*out = NULL;
	return (slot);
}
