#include "obj_int.h"
#include "velum/libk.h"

static t_grave	g_grave;

void	ipc_grave_init(void)
{
	memset(&g_grave, 0, sizeof(t_grave));
	spin_init(&g_grave.lock, "grave");
}

void	ipc_bury(t_msgq *q)
{
	uint64_t	fl;

	if (!q->head)
		return ;
	fl = spin_lock_irqsave(&g_grave.lock);
	if (g_grave.q.tail)
		g_grave.q.tail->next = q->head;
	else
		g_grave.q.head = q->head;
	g_grave.q.tail = q->tail;
	g_grave.q.count += q->count;
	spin_unlock_irqrestore(&g_grave.lock, fl);
	q->head = NULL;
	q->tail = NULL;
	q->count = 0;
}

void	ipc_reap(void)
{
	uint64_t	fl;
	t_ipcmsg	*m;

	fl = spin_lock_irqsave(&g_grave.lock);
	if (g_grave.active)
	{
		spin_unlock_irqrestore(&g_grave.lock, fl);
		return ;
	}
	g_grave.active = 1;
	m = msgq_pop(&g_grave.q);
	while (m)
	{
		spin_unlock_irqrestore(&g_grave.lock, fl);
		msg_free(m);
		fl = spin_lock_irqsave(&g_grave.lock);
		m = msgq_pop(&g_grave.q);
	}
	g_grave.active = 0;
	spin_unlock_irqrestore(&g_grave.lock, fl);
}
