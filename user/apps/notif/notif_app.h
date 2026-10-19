#ifndef NOTIF_APP_H
# define NOTIF_APP_H

# include <stdint.h>
# include "velum/notif.h"
# include "../common/uiwin.h"

# define NA_CLIENTS_MAX 16
# define NA_W 380
# define NA_MARGIN_X 12
# define NA_MARGIN_TOP 12
# define NA_MARGIN_BOTTOM 80
# define NA_HEAD_H 44
# define NA_CARD_H 92
# define NA_CARD_GAP 8
# define NA_VISIBLE_MAX 5
# define NA_TICK_NS 100000000ull
# define NA_AGE_TICKS 10
# define NA_ID_CLEAR 100
# define NA_ID_EMPTY 101
# define NA_ID_HEAD 102
# define NA_ID_CARD 200
# define NA_ID_STRIDE 4
# define NA_ID_DISMISS 300
# define NA_TITLE "Notifications"
# define NA_ARG_OPEN "ouvrir"

typedef struct s_notifapp
{
	t_notifstore	store;
	t_handle		listener;
	t_handle		clients[NA_CLIENTS_MAX];
	t_uiwin			ui;
	t_handle		timer;
	int32_t			screen_h;
	uint32_t		ticks;
	int				visible;
	int				dirty;
	int				quit;
}	t_notifapp;

int		notifapp_open(t_notifapp *app, int open_now);
void	notifapp_run(t_notifapp *app);
void	notifapp_close(t_notifapp *app);

int		notifsrv_start(t_notifapp *app);
void	notifsrv_poll(t_notifapp *app);
void	notifsrv_stop(t_notifapp *app);
void	notifsrv_handle(t_notifapp *app, t_notifmsg *m);

void	notifui_show(t_notifapp *app, int visible);
void	notifui_toggle(t_notifapp *app);
int		notifui_rebuild(t_notifapp *app);
void	notifui_command(void *c, uint32_t code, void *user);

#endif
