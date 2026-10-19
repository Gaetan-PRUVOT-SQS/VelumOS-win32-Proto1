#include "velum/err.h"
#include "velum/libk.h"
#include "notif_app.h"
#include "../common/platform.h"

static int	post(t_notifapp *app, t_notifmsg *m)
{
	t_notif	n;
	int		r;

	memset(&n, 0, sizeof(n));
	strlcpy(n.app, m->app, sizeof(n.app));
	strlcpy(n.title, m->title, sizeof(n.title));
	strlcpy(n.text, m->text, sizeof(n.text));
	n.icon = m->icon;
	n.stamp_ns = os_mono_ns();
	r = notif_store_post(&app->store, &n);
	if (r < 0)
		return (r);
	m->id = (uint32_t)r;
	app->dirty = 1;
	return (0);
}

static int	apply(t_notifapp *app, t_notifmsg *m)
{
	int	r;

	if (m->type == NOTIFC_POST)
		return (post(app, m));
	if (m->type == NOTIFC_TOGGLE)
		notifui_toggle(app);
	else if (m->type == NOTIFC_DISMISS)
	{
		r = notif_store_dismiss(&app->store, m->id);
		app->dirty = 1;
		return (r);
	}
	else if (m->type == NOTIFC_CLEAR)
	{
		notif_store_clear(&app->store);
		app->dirty = 1;
	}
	return (0);
}

void	notifsrv_handle(t_notifapp *app, t_notifmsg *m)
{
	uint32_t	type;
	int			r;

	type = m->type;
	r = apply(app, m);
	m->count = notif_store_count(&app->store);
	m->status = r;
	m->type = NOTIFS_OK;
	if (type == NOTIFC_COUNT)
		m->type = NOTIFS_COUNT;
	if (r < 0)
		m->type = NOTIFS_ERROR;
}
