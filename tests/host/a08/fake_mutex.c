#include <stdio.h>
#include <stdlib.h>
#include "fake.h"

void	mutex_init(t_mutex *m, const char *name)
{
	m->owner = NULL;
	m->name = name;
}

void	mutex_lock(t_mutex *m)
{
	if (m->owner)
	{
		fprintf(stderr, "mutex deja pris : %s\n", m->name);
		abort();
	}
	m->owner = (struct s_thread *)m;
}

bool	mutex_trylock(t_mutex *m)
{
	if (m->owner)
		return (false);
	m->owner = (struct s_thread *)m;
	return (true);
}

void	mutex_unlock(t_mutex *m)
{
	if (!m->owner)
	{
		fprintf(stderr, "mutex rendu sans etre pris : %s\n", m->name);
		abort();
	}
	m->owner = NULL;
}

int	fk_vmm_refuse(void)
{
	fk_vmq_fire(FK_VMQ_ON_FAIL, 1);
	return (E_NOMEM);
}
