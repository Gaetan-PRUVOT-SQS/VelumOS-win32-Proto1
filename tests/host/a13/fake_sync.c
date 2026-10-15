#include "fakes.h"

void	mutex_init(t_mutex *m, const char *name)
{
	memset(m, 0, sizeof(*m));
	m->name = name;
}

void	mutex_lock(t_mutex *m)
{
	if (m->owner)
		g_fp.bad_locks++;
	m->owner = (struct s_thread *)&g_fp;
	g_fp.locks++;
}

void	mutex_unlock(t_mutex *m)
{
	if (!m->owner)
		g_fp.bad_locks++;
	m->owner = NULL;
	g_fp.unlocks++;
}
