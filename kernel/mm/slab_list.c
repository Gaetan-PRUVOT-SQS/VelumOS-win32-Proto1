#include "heap_int.h"

void	slabq_push_head(t_slabq *q, t_slab *s)
{
	s->prev = NULL;
	s->next = q->head;
	if (q->head)
		q->head->prev = s;
	else
		q->tail = s;
	q->head = s;
}

void	slabq_push_tail(t_slabq *q, t_slab *s)
{
	s->next = NULL;
	s->prev = q->tail;
	if (q->tail)
		q->tail->next = s;
	else
		q->head = s;
	q->tail = s;
}

void	slabq_remove(t_slabq *q, t_slab *s)
{
	if (s->prev)
		s->prev->next = s->next;
	else
		q->head = s->next;
	if (s->next)
		s->next->prev = s->prev;
	else
		q->tail = s->prev;
	s->next = NULL;
	s->prev = NULL;
}
