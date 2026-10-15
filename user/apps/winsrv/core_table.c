#include "velum/err.h"
#include "velum/libk.h"
#include "ws_core.h"

void	wt_init(t_wtable *t, t_rect screen)
{
	memset(t, 0, sizeof(*t));
	t->active = -1;
	t->screen = screen;
	t->work = screen;
	t->gen = 1;
}

int	wt_alloc(t_wtable *t, int32_t owner)
{
	int	i;

	i = 0;
	while (i < WS_WIN_MAX && t->w[i].id != 0)
		i++;
	if (i == WS_WIN_MAX)
		return (E_NOSPC);
	memset(&t->w[i], 0, sizeof(t->w[i]));
	t->gen = (t->gen + 1) & 0xffffff;
	if (t->gen == 0)
		t->gen = 1;
	t->w[i].id = (t->gen << 8) | (uint32_t)(i + 1);
	t->w[i].owner = owner;
	t->nused++;
	return (i);
}

int	wt_find(const t_wtable *t, uint32_t id)
{
	uint32_t	slot;

	slot = id & 0xff;
	if (id == 0 || slot == 0 || slot > WS_WIN_MAX)
		return (-1);
	if (t->w[slot - 1].id != id)
		return (-1);
	return ((int)slot - 1);
}

uint32_t	wt_count_owner(const t_wtable *t, int32_t owner)
{
	uint32_t	n;
	int			i;

	n = 0;
	i = 0;
	while (i < WS_WIN_MAX)
	{
		if (t->w[i].id != 0 && t->w[i].owner == owner)
			n++;
		i++;
	}
	return (n);
}

void	wt_free(t_wtable *t, int slot)
{
	if (slot < 0 || slot >= WS_WIN_MAX || t->w[slot].id == 0)
		return ;
	wz_remove(t, slot);
	if (t->active == slot)
		t->active = -1;
	memset(&t->w[slot], 0, sizeof(t->w[slot]));
	t->nused--;
}
