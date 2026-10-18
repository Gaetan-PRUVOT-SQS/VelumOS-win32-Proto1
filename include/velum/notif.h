#ifndef NOTIF_H
# define NOTIF_H

# include <stddef.h>
# include <stdint.h>

# define NOTIF_PORT_NAME "notif"
# define NOTIF_MAGIC 0x314e5456u
# define NOTIF_MAX 16
# define NOTIF_APP_MAX 24
# define NOTIF_TITLE_MAX 48
# define NOTIF_TEXT_MAX 120
# define NOTIF_BADGE_MAX 4
# define NOTIF_AGE_MAX 24
# define NOTIF_CAP_BADGE 99

# define NOTIFC_POST 1
# define NOTIFC_COUNT 2
# define NOTIFC_TOGGLE 3
# define NOTIFC_DISMISS 4
# define NOTIFC_CLEAR 5

# define NOTIFS_OK 0x101
# define NOTIFS_COUNT 0x102
# define NOTIFS_ERROR 0x1ff

typedef struct s_notif
{
	uint32_t	id;
	uint32_t	icon;
	uint64_t	stamp_ns;
	char		app[NOTIF_APP_MAX];
	char		title[NOTIF_TITLE_MAX];
	char		text[NOTIF_TEXT_MAX];
}	t_notif;

typedef struct s_notifstore
{
	t_notif		items[NOTIF_MAX];
	uint32_t	count;
	uint32_t	next_id;
	uint32_t	dropped;
}	t_notifstore;

typedef struct s_notifmsg
{
	uint32_t	magic;
	uint32_t	type;
	uint32_t	size;
	uint32_t	seq;
	int32_t		status;
	uint32_t	id;
	uint32_t	count;
	uint32_t	icon;
	uint64_t	stamp_ns;
	char		app[NOTIF_APP_MAX];
	char		title[NOTIF_TITLE_MAX];
	char		text[NOTIF_TEXT_MAX];
}	t_notifmsg;

void		notif_store_init(t_notifstore *s);
int			notif_store_post(t_notifstore *s, const t_notif *n);
int			notif_store_dismiss(t_notifstore *s, uint32_t id);
void		notif_store_clear(t_notifstore *s);
uint32_t	notif_store_count(const t_notifstore *s);
const t_notif	*notif_store_get(const t_notifstore *s, uint32_t index);

int			notif_fields_valid(const char *app, const char *title,
				const char *text);
int			notif_msg_decode(const void *buf, uint32_t len, t_notifmsg *out);
void		notif_msg_init(t_notifmsg *m, uint32_t type);
int			notif_msg_post(t_notifmsg *m, const char *app, const char *title,
				const char *text, uint32_t icon);

size_t		notif_badge_text(uint32_t count, char *out, size_t size);
size_t		notif_age_text(uint64_t now_ns, uint64_t stamp_ns, char *out,
				size_t size);

int			notif_post(const char *app, const char *title, const char *text,
				uint32_t icon);
int			notif_client_count(uint32_t *out);
int			notif_client_toggle(void);
int			notif_client_dismiss(uint32_t id);
int			notif_client_clear(void);
void		notif_client_close(void);

#endif
