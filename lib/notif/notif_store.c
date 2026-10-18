#include <limits.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/notif.h"

void	notif_store_init(t_notifstore *s)
{
	if (s == NULL)
		return ;
	memset(s, 0, sizeof(*s));
	s->next_id = 1;
}

static void	drop_oldest(t_notifstore *s)
{
	memmove(&s->items[0], &s->items[1], (NOTIF_MAX - 1) * sizeof(t_notif));
	s->count--;
	s->dropped++;
}

static int	id_live(const t_notifstore *s, uint32_t id)
{
	uint32_t	i;

	i = 0;
	while (i < s->count)
	{
		if (s->items[i].id == id)
			return (1);
		i++;
	}
	return (0);
}

static uint32_t	id_fit(uint32_t id)
{
	if (id == 0 || id > (uint32_t)INT_MAX)
		return (1);
	return (id);
}

static void	next_id(t_notifstore *s, t_notif *slot)
{
	uint32_t	id;
	uint32_t	steps;

	id = id_fit(s->next_id);
	steps = 0;
	while (id_live(s, id) && steps < NOTIF_MAX)
	{
		if (id == (uint32_t)INT_MAX)
			id = 1;
		else
			id++;
		steps++;
	}
	slot->id = id;
	if (id == (uint32_t)INT_MAX)
		s->next_id = 1;
	else
		s->next_id = id + 1;
}

int	notif_store_post(t_notifstore *s, const t_notif *n)
{
	t_notif	*slot;

	if (s == NULL || n == NULL)
		return (E_INVAL);
	if (!notif_fields_valid(n->app, n->title, n->text))
		return (E_INVAL);
	if (s->count == NOTIF_MAX)
		drop_oldest(s);
	slot = &s->items[s->count];
	*slot = *n;
	next_id(s, slot);
	s->count++;
	return ((int)slot->id);
}

int	notif_store_dismiss(t_notifstore *s, uint32_t id)
{
	uint32_t	i;

	if (s == NULL || id == 0)
		return (E_INVAL);
	i = 0;
	while (i < s->count && s->items[i].id != id)
		i++;
	if (i == s->count)
		return (E_NOENT);
	memmove(&s->items[i], &s->items[i + 1],
		(s->count - i - 1) * sizeof(t_notif));
	s->count--;
	memset(&s->items[s->count], 0, sizeof(t_notif));
	return (0);
}

void	notif_store_clear(t_notifstore *s)
{
	uint32_t	keep;

	if (s == NULL)
		return ;
	keep = s->next_id;
	memset(s->items, 0, sizeof(s->items));
	s->count = 0;
	s->next_id = keep;
}

uint32_t	notif_store_count(const t_notifstore *s)
{
	if (s == NULL)
		return (0);
	return (s->count);
}

const t_notif	*notif_store_get(const t_notifstore *s, uint32_t index)
{
	if (s == NULL || index >= s->count)
		return (NULL);
	return (&s->items[s->count - 1 - index]);
}
