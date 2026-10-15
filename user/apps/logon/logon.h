#ifndef LOGON_H
# define LOGON_H

# include <stdint.h>
# include "velum/ctl.h"
# include "velum/wm.h"
# include "../common/uiwin.h"
# include "accounts.h"
# include "auth.h"
# include "limiter.h"
# include "logon_flow.h"
# include "logon_layout.h"

# define LOGON_SHELL_PATH "/system/bin/shell"
# define LOGON_ID_TILE 100
# define LOGON_ID_PW 200
# define LOGON_ID_GO 201
# define LOGON_ID_MSG 210
# define LOGON_ID_SHUT 220
# define LOGON_ID_DLG_TEXT 230
# define LOGON_PW_LIMIT 128

typedef struct s_logon
{
	t_wmhello		info;
	t_uiwin			ui;
	t_logonlayout	lay;
	t_accounts		set;
	t_limiters		lim;
	t_logonflow		flow;
	uint32_t		shown[LOGON_TILES_MAX];
	uint32_t		nshown;
	t_handle		timer;
	t_handle		session;
	int				act;
	int				quit;
}	t_logon;

int			logon_boot(t_logon *lg);
int			logon_load(t_logon *lg);
int			logon_window(t_logon *lg);
int			logon_build(t_logon *lg);
int			logon_add_dialog(t_logon *lg);
t_ctl		*logon_add_ctl(t_logon *lg, t_ctlspec s, t_fontid font);
void		logon_sync(t_logon *lg);
void		logon_overlay(void *user, t_rect dirty);
void		logon_on_command(void *c, uint32_t code, void *user);
void		logon_pick(t_logon *lg, uint32_t tile);
void		logon_submit(t_logon *lg);
void		logon_after(t_logon *lg, int act);
void		logon_do_act(t_logon *lg);
void		logon_tick(t_logon *lg);
void		logon_event(t_logon *lg, const t_uimsg *m);
void		logon_start_session(t_logon *lg);
uint32_t	logon_remaining(const t_logon *lg);

#endif
