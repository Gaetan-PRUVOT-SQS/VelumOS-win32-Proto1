#include "inp_queue.h"
#include "velum/libk.h"

t_reader	*reader_acquire(uint32_t kind)
{
	uint64_t	flags;
	uint32_t	i;
	t_reader	*r;

	r = 0;
	flags = spin_lock_irqsave(&g_inputq.lock);
	i = 0;
	while (i < INPQ_READERS && !r)
	{
		if (!g_inputq.readers[i].used)
			r = &g_inputq.readers[i];
		i++;
	}
	if (r)
	{
		memset(r, 0, sizeof(*r));
		r->used = 1;
		r->kind = kind;
		r->cur = g_inputq.head;
	}
	spin_unlock_irqrestore(&g_inputq.lock, flags);
	return (r);
}

void	reader_attach(t_reader *r, t_object *obj)
{
	uint64_t	flags;

	flags = spin_lock_irqsave(&g_inputq.lock);
	r->obj = obj;
	spin_unlock_irqrestore(&g_inputq.lock, flags);
}

void	reader_release(t_reader *r)
{
	uint64_t	flags;
	uint32_t	i;
	uint32_t	open;

	flags = spin_lock_irqsave(&g_inputq.lock);
	memset(r, 0, sizeof(*r));
	open = 0;
	i = 0;
	while (i < INPQ_READERS)
	{
		open += g_inputq.readers[i].used;
		i++;
	}
	if (open == 0)
	{
		memset(g_inputq.ring, 0, sizeof(g_inputq.ring));
		g_inputq.kcur = g_inputq.head;
	}
	spin_unlock_irqrestore(&g_inputq.lock, flags);
}

uint32_t	reader_take(t_reader *r, t_inpevent *dst, uint32_t max)
{
	uint64_t	flags;
	uint64_t	cur;
	uint32_t	n;

	flags = spin_lock_irqsave(&g_inputq.lock);
	cur = r->cur;
	n = 0;
	while (cur != g_inputq.head && n < max)
	{
		if (inp_kind_match(r->kind, g_inputq.ring[cur & INPQ_MASK].type))
		{
			dst[n] = g_inputq.ring[cur & INPQ_MASK];
			n++;
		}
		cur++;
	}
	__atomic_store_n(&r->cur, cur, __ATOMIC_RELEASE);
	spin_unlock_irqrestore(&g_inputq.lock, flags);
	return (n);
}

void	reader_notify(t_inputq *q, t_reader *r, uint32_t type, uint32_t victim)
{
	if (q->head - r->cur > INPQ_LEN)
	{
		__atomic_store_n(&r->cur, q->head - INPQ_LEN, __ATOMIC_RELEASE);
		if (inp_kind_match(r->kind, victim))
		{
			r->lost++;
			q->lost++;
		}
	}
	if (r->obj && inp_kind_match(r->kind, type))
		obj_signal(r->obj);
}
