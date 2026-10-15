#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"

t_ipcmsg	*msg_alloc(uint32_t len)
{
	t_ipcmsg	*m;

	if (len > IPC_MSG_MAX)
		return (NULL);
	m = kmalloc_tag(sizeof(t_ipcmsg) + len, HEAP_IPC);
	if (!m)
		return (NULL);
	memset(m, 0, sizeof(t_ipcmsg));
	m->len = len;
	return (m);
}

void	msg_free(t_ipcmsg *m)
{
	uint32_t	i;

	if (!m)
		return ;
	i = 0;
	while (i < m->nh)
	{
		obj_unref(m->objs[i]);
		i++;
	}
	msg_uncharge(m);
	kfree(m);
}

int	msgq_push(t_msgq *q, t_ipcmsg *m, uint32_t max)
{
	if (q->count >= max)
		return (E_AGAIN);
	m->next = NULL;
	if (q->tail)
		q->tail->next = m;
	else
		q->head = m;
	q->tail = m;
	__atomic_store_n(&q->count, q->count + 1, __ATOMIC_RELEASE);
	return (0);
}

void	msgq_push_front(t_msgq *q, t_ipcmsg *m)
{
	m->next = q->head;
	q->head = m;
	if (!q->tail)
		q->tail = m;
	__atomic_store_n(&q->count, q->count + 1, __ATOMIC_RELEASE);
}

t_ipcmsg	*msgq_pop(t_msgq *q)
{
	t_ipcmsg	*m;

	m = q->head;
	if (!m)
		return (NULL);
	q->head = m->next;
	if (!q->head)
		q->tail = NULL;
	__atomic_store_n(&q->count, q->count - 1, __ATOMIC_RELEASE);
	m->next = NULL;
	return (m);
}
