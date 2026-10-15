#include <stdlib.h>
#include "velum/err.h"
#include "fake_kern.h"
#include "ws_sys.h"

void	ws_sys_log(const char *msg)
{
	g_fk.logs++;
	g_fk.last_log = msg;
}

int64_t	fk_connect(void)
{
	int		a;
	int		b;
	t_fkobj	*l;

	if (g_fk.listener < 0)
		return (E_NOENT);
	l = &g_fk.o[g_fk.listener];
	if (l->npend >= FK_LISTEN_Q)
		return (E_AGAIN);
	a = fk_obj_new(FK_CHAN);
	if (a < 0)
		return (a);
	b = fk_obj_new(FK_CHAN);
	if (b < 0)
	{
		fk_unref(a);
		return (b);
	}
	g_fk.o[a].peer = b;
	g_fk.o[b].peer = a;
	l->pend[l->npend++] = b;
	return (fk_handle_new(a));
}
