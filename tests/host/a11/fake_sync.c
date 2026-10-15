#include <stdlib.h>
#include "fake.h"
#include "velum/sync.h"

void	mutex_init(t_mutex *m, const char *name)
{
	m->name = name;
	m->owner = NULL;
}

void	mutex_lock(t_mutex *m)
{
	if (m->owner)
		abort();
	m->owner = (struct s_thread *)m;
	g_fk.locks++;
}

void	mutex_unlock(t_mutex *m)
{
	if (!m->owner || g_fk.locks <= 0)
		abort();
	m->owner = NULL;
	g_fk.locks--;
}

bool	fk_should_fail(void)
{
	if (!g_fk.armed)
		return (false);
	if (g_fk.fail_after == 0)
		return (true);
	g_fk.fail_after--;
	return (false);
}

int	fk_live(void)
{
	return (g_fk.live);
}
