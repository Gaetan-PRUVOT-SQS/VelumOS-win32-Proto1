#include <stdlib.h>
#include <string.h>
#include "velum/err.h"
#include "fake_kern.h"

t_fk	g_fk;

int	fk_obj_new(int32_t type)
{
	int	i;

	i = 0;
	while (i < FK_OBJ_MAX && g_fk.o[i].type != FK_FREE)
		i++;
	if (i == FK_OBJ_MAX)
		return (E_NOMEM);
	memset(&g_fk.o[i], 0, sizeof(g_fk.o[i]));
	g_fk.o[i].type = type;
	g_fk.o[i].refs = 1;
	g_fk.o[i].peer = -1;
	return (i);
}

int64_t	fk_handle_new(int obj)
{
	int	i;

	i = 0;
	while (i < FK_HANDLE_MAX && g_fk.h[i] != 0)
		i++;
	if (i == FK_HANDLE_MAX)
		return (E_MFILE);
	g_fk.h[i] = obj + 1;
	return (i + 1);
}

int	fk_obj_of(t_handle h, int32_t type)
{
	int	obj;

	if (h == 0 || h > FK_HANDLE_MAX || g_fk.h[h - 1] == 0)
		return (E_BADF);
	obj = g_fk.h[h - 1] - 1;
	if (type != FK_FREE && g_fk.o[obj].type != type)
		return (E_BADF);
	return (obj);
}

static void	destroy(int obj)
{
	t_fkobj	*o;
	t_fkmsg	*m;

	o = &g_fk.o[obj];
	while (o->n > 0)
	{
		m = &o->q[o->head];
		free(m->data);
		if (m->obj >= 0)
			fk_unref(m->obj);
		o->head = (o->head + 1) % IPC_QUEUE_MAX;
		o->n--;
	}
	while (o->npend > 0)
		fk_unref(o->pend[--o->npend]);
	if (o->peer >= 0)
		g_fk.o[o->peer].peer = -1;
	if (obj == g_fk.listener)
		g_fk.listener = -1;
	free(o->mem);
	memset(o, 0, sizeof(*o));
}

void	fk_unref(int obj)
{
	t_fkobj	*o;

	o = &g_fk.o[obj];
	if (o->type == FK_FREE)
		return ;
	if (o->refs > 0)
		o->refs--;
	if (o->refs == 0 && o->maps == 0)
		destroy(obj);
}
