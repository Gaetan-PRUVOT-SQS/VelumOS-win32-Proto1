#include <stdio.h>
#include <stdlib.h>
#include "fake.h"

void	spin_init(t_spinlock *l, const char *name)
{
	l->ticket = 0;
	l->serving = 0;
	l->name = name;
}

uint64_t	spin_lock_irqsave(t_spinlock *l)
{
	if (l->ticket)
	{
		fprintf(stderr, "verrou deja pris : %s\n", l->name);
		abort();
	}
	l->ticket = 1;
	g_fk.depth++;
	return (0);
}

void	spin_unlock_irqrestore(t_spinlock *l, uint64_t flags)
{
	(void)flags;
	if (!l->ticket)
	{
		fprintf(stderr, "verrou rendu sans etre pris : %s\n", l->name);
		abort();
	}
	l->ticket = 0;
	g_fk.depth--;
	if (g_fk.depth == 0 && g_fk.hook)
		fk_hook_run(0);
}

void	waitq_init(t_waitq *wq)
{
	wq->head = NULL;
	wq->tail = NULL;
	spin_init(&wq->lock, "waitq");
}

void	waitq_wake_all(t_waitq *wq)
{
	wq->head = NULL;
}
