#ifndef UIWIN_H
# define UIWIN_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/ctl.h"
# include "velum/wm.h"

# define UIE_NONE 0
# define UIE_USED 1
# define UIE_CLOSE 2
# define UI_WAIT_MSG 0
# define UI_WAIT_TIMER 1
# define UI_CONNECT_TRIES 100
# define UI_CONNECT_STEP_NS 100000000ull

typedef struct s_uimsg
{
	uint64_t	words[WM_MSG_MAX / 8];
	int			len;
}	t_uimsg;

typedef struct s_uiwin
{
	t_wmwin		win;
	t_surface	view;
	t_rect		area;
	t_ctlroot	root;
	void		(*underlay)(void *user, t_rect dirty);
	void		*underlay_user;
	void		(*overlay)(void *user, t_rect dirty);
	void		*overlay_user;
	int			ready;
}	t_uiwin;

void	ui_request(t_wmcreate *rq, t_rect r, uint32_t style, const char *title);
int		uiwin_open(t_uiwin *u, const t_wmcreate *rq, t_ctlcb command);
int		uiwin_area(t_uiwin *u, t_rect area);
void	uiwin_close(t_uiwin *u);
void	uiwin_flush(t_uiwin *u);
void	uiwin_resized(t_uiwin *u);
int		uiwin_event(t_uiwin *u, const t_uimsg *m);
int		ui_next(t_uimsg *m);
int		ui_connect(t_wmhello *info);
int		ui_wait(t_handle timer, uint64_t timeout_ns);

#endif
