#include <stddef.h>
#include "velum/sync.h"

void	mutex_init(t_mutex *m, const char *name)
{
	spin_init(&m->lock, name);
	m->owner = NULL;
	m->name = name;
}

void	mutex_lock(t_mutex *m)
{
	spin_lock(&m->lock);
}

void	mutex_unlock(t_mutex *m)
{
	spin_unlock(&m->lock);
}
