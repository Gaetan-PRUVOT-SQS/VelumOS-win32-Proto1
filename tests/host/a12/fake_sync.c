#include <stdio.h>
#include <stdlib.h>
#include "velum/sync.h"

static void	lock_take(t_spinlock *l, const char *what)
{
	if (l->ticket != 0)
	{
		fprintf(stderr, "verrou %s pris deux fois\n", what);
		abort();
	}
	l->ticket = 1;
}

static void	lock_give(t_spinlock *l, const char *what)
{
	if (l->ticket != 1)
	{
		fprintf(stderr, "verrou %s rendu sans être pris\n", what);
		abort();
	}
	l->ticket = 0;
}

void	spin_init(t_spinlock *l, const char *name)
{
	l->ticket = 0;
	l->serving = 0;
	l->name = name;
}

void	spin_lock(t_spinlock *l)
{
	lock_take(l, l->name);
}

void	spin_unlock(t_spinlock *l)
{
	lock_give(l, l->name);
}
