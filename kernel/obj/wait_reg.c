#include "obj_int.h"
#include "velum/libk.h"

t_wreg	g_wreg;

uint32_t	wreg_bucket(const t_object *o)
{
	uint64_t	v;

	v = (uint64_t)(uintptr_t)o;
	v ^= v >> 17;
	v *= 0x9e3779b97f4a7c15ull;
	return ((uint32_t)(v >> WREG_SHIFT));
}

void	wreg_init(void)
{
	memset(&g_wreg, 0, sizeof(t_wreg));
	spin_init(&g_wreg.lock, "wait-reg");
}

void	wreg_add(t_waitset *ws)
{
	uint64_t	fl;
	uint32_t	i;
	uint32_t	b;
	t_wlink		*l;

	fl = spin_lock_irqsave(&g_wreg.lock);
	i = 0;
	while (i < ws->n)
	{
		l = &ws->links[i];
		b = wreg_bucket(l->obj);
		l->prev = NULL;
		l->next = g_wreg.heads[b];
		if (l->next)
			l->next->prev = l;
		g_wreg.heads[b] = l;
		i++;
	}
	spin_unlock_irqrestore(&g_wreg.lock, fl);
}

void	wreg_del(t_waitset *ws)
{
	uint64_t	fl;
	uint32_t	i;
	t_wlink		*l;

	fl = spin_lock_irqsave(&g_wreg.lock);
	i = 0;
	while (i < ws->n)
	{
		l = &ws->links[i];
		if (l->prev)
			l->prev->next = l->next;
		else
			g_wreg.heads[wreg_bucket(l->obj)] = l->next;
		if (l->next)
			l->next->prev = l->prev;
		l->next = NULL;
		l->prev = NULL;
		i++;
	}
	spin_unlock_irqrestore(&g_wreg.lock, fl);
}

void	wreg_notify(t_object *o)
{
	uint64_t	fl;
	t_wlink		*l;

	fl = spin_lock_irqsave(&g_wreg.lock);
	l = g_wreg.heads[wreg_bucket(o)];
	while (l)
	{
		if (l->obj == o)
			wblock_fire(l->wb);
		l = l->next;
	}
	spin_unlock_irqrestore(&g_wreg.lock, fl);
}
