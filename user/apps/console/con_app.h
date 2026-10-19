#ifndef CON_APP_H
# define CON_APP_H

# include "../common/uiwin.h"
# include "console.h"

# define CON_JOBS_MAX 8
# define CON_WIN_X 200
# define CON_WIN_Y 70
# define CON_WIN_W 880
# define CON_WIN_H 560
# define CON_PAD_X 24
# define CON_PAD_Y 14
# define CON_LINE_H 22
# define CON_STATUS_H 32
# define CON_BLINK_NS 500000000ull
# define CON_TITLE "Console"

typedef struct s_conjobs
{
	t_handle	handle[CON_JOBS_MAX];
	uint32_t	pid[CON_JOBS_MAX];
	uint32_t	count;
}	t_conjobs;

typedef struct s_conapp
{
	t_uiwin		ui;
	t_console	con;
	t_conjobs	jobs;
	t_handle	timer;
	int32_t		cell_w;
	uint32_t	rows_vis;
	uint32_t	scroll;
	int			blink;
	int			quit;
}	t_conapp;

extern const t_conops	g_conops_system;

void	con_jobs_reap(t_conjobs *j);
void	con_paint(t_conapp *a);
void	con_layout(t_conapp *a);
void	con_on_key(t_conapp *a, const t_inpevent *ev);
void	con_dispatch(t_conapp *a, const t_uimsg *m);
int		con_open(t_conapp *a);
void	con_loop(t_conapp *a);

#endif
