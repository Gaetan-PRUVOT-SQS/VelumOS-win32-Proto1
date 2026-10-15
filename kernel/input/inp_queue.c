#include "inp_queue.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/timer.h"

t_inputq	g_inputq;

void	inpq_init(void)
{
	memset(&g_inputq, 0, sizeof(g_inputq));
	spin_init(&g_inputq.lock, "input_queue");
}

static void	inpq_trim(t_inputq *q, uint32_t type, uint32_t victim)
{
	uint32_t	i;

	if (!q->kuse)
		q->kcur = q->head;
	else if (q->head - q->kcur > INPQ_LEN)
	{
		q->kcur = q->head - INPQ_LEN;
		q->lost++;
	}
	i = 0;
	while (i < INPQ_READERS)
	{
		if (q->readers[i].used)
			reader_notify(q, &q->readers[i], type, victim);
		i++;
	}
}

void	input_push(const t_inpevent *ev)
{
	t_inpevent	copy;
	uint64_t	flags;
	uint32_t	victim;

	if (!ev)
		return ;
	copy = *ev;
	if (copy.time_ns == 0)
		copy.time_ns = time_now_ns();
	if (g_inputq.logging)
		inplog_event(&copy);
	flags = spin_lock_irqsave(&g_inputq.lock);
	victim = g_inputq.ring[g_inputq.head & INPQ_MASK].type;
	g_inputq.ring[g_inputq.head & INPQ_MASK] = copy;
	__atomic_store_n(&g_inputq.head, g_inputq.head + 1, __ATOMIC_RELEASE);
	inpq_trim(&g_inputq, copy.type, victim);
	spin_unlock_irqrestore(&g_inputq.lock, flags);
}

int	input_pop(t_inpevent *ev)
{
	uint64_t	flags;
	int			got;

	if (!ev)
		return (E_INVAL);
	flags = spin_lock_irqsave(&g_inputq.lock);
	g_inputq.kuse = 1;
	got = 0;
	if (g_inputq.kcur != g_inputq.head)
	{
		*ev = g_inputq.ring[g_inputq.kcur & INPQ_MASK];
		g_inputq.kcur++;
		got = 1;
	}
	spin_unlock_irqrestore(&g_inputq.lock, flags);
	return (got);
}

uint64_t	input_lost(void)
{
	uint64_t	flags;
	uint64_t	total;

	flags = spin_lock_irqsave(&g_inputq.lock);
	total = g_inputq.lost;
	spin_unlock_irqrestore(&g_inputq.lock, flags);
	return (total);
}
