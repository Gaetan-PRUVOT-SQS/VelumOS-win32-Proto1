#include "velum/libk.h"
#include "tasklist.h"

void	tl_init(t_tasklist *l)
{
	memset(l, 0, sizeof(*l));
}

int32_t	tl_find(const t_tasklist *l, uint32_t id)
{
	uint32_t	i;

	i = 0;
	while (i < l->count)
	{
		if (l->list[i].id == id)
			return ((int32_t)i);
		i++;
	}
	return (-1);
}

int	tl_shown(uint32_t style, uint32_t state)
{
	uint32_t	hidden_styles;

	hidden_styles = WS_TOOLWINDOW | WS_DESKTOP | WS_APPBAR | WS_POPUP
		| WS_NOACTIVATE;
	return ((style & hidden_styles) == 0 && state != WSTATE_HIDDEN);
}

void	tl_remove(t_tasklist *l, uint32_t id)
{
	int32_t	i;

	i = tl_find(l, id);
	if (i < 0)
		return ;
	l->count--;
	memmove(&l->list[i], &l->list[i + 1],
		(l->count - (uint32_t)i) * sizeof(l->list[0]));
	memset(&l->list[l->count], 0, sizeof(l->list[0]));
}

const t_task	*tl_active(const t_tasklist *l)
{
	uint32_t	i;

	i = 0;
	while (i < l->count)
	{
		if (l->list[i].active)
			return (&l->list[i]);
		i++;
	}
	return (NULL);
}
