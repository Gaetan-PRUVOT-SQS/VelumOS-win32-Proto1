#include "velum/err.h"
#include "velum/libk.h"
#include "velum/notif.h"

static int	field_valid(const char *f, size_t max, int empty_ok)
{
	size_t	n;
	size_t	i;

	n = strnlen(f, max);
	if (n == max)
		return (0);
	if (n == 0)
		return (empty_ok);
	i = 0;
	while (i < n)
	{
		if ((unsigned char)f[i] < 0x20 || (unsigned char)f[i] == 0x7f)
			return (0);
		i++;
	}
	return (1);
}

int	notif_fields_valid(const char *app, const char *title, const char *text)
{
	if (app == NULL || title == NULL || text == NULL)
		return (0);
	return (field_valid(app, NOTIF_APP_MAX, 0)
		&& field_valid(title, NOTIF_TITLE_MAX, 0)
		&& field_valid(text, NOTIF_TEXT_MAX, 1));
}

static int	type_known(uint32_t type)
{
	return (type >= NOTIFC_POST && type <= NOTIFC_CLEAR);
}

int	notif_msg_decode(const void *buf, uint32_t len, t_notifmsg *out)
{
	if (buf == NULL || out == NULL || len != sizeof(*out))
		return (E_INVAL);
	memcpy(out, buf, sizeof(*out));
	if (out->magic != NOTIF_MAGIC || out->size != sizeof(*out))
		return (E_INVAL);
	if (!type_known(out->type))
		return (E_INVAL);
	if (out->type == NOTIFC_POST
		&& !notif_fields_valid(out->app, out->title, out->text))
		return (E_INVAL);
	if (out->type == NOTIFC_DISMISS && out->id == 0)
		return (E_INVAL);
	return (0);
}

void	notif_msg_init(t_notifmsg *m, uint32_t type)
{
	memset(m, 0, sizeof(*m));
	m->magic = NOTIF_MAGIC;
	m->type = type;
	m->size = sizeof(*m);
}

int	notif_msg_post(t_notifmsg *m, const char *app, const char *title,
	const char *text, uint32_t icon)
{
	if (m == NULL || !notif_fields_valid(app, title, text))
		return (E_INVAL);
	notif_msg_init(m, NOTIFC_POST);
	strlcpy(m->app, app, sizeof(m->app));
	strlcpy(m->title, title, sizeof(m->title));
	strlcpy(m->text, text, sizeof(m->text));
	m->icon = icon;
	return (0);
}
