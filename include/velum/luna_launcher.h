#ifndef LUNA_LAUNCHER_H
# define LUNA_LAUNCHER_H

# include <stdint.h>
# include "velum/luna.h"

# define LUNA_LAUNCHER_WIDTH 540
# define LUNA_LAUNCHER_HEIGHT 289
# define LUNA_LAUNCHER_MARGIN_X 12
# define LUNA_LAUNCHER_TILES 4
# define LUNA_LAUNCHER_NONE -1
# define LUNA_LAUNCHER_LOGOFF 4
# define LUNA_LAUNCHER_POWER 5

typedef struct s_launchertile
{
	const char	*label;
	t_iconid	icon;
}	t_launchertile;

typedef struct s_launcher
{
	const char				*user;
	const t_launchertile	*tiles;
	int32_t					ntiles;
	int32_t					hot;
	int32_t					focus;
}	t_launcher;

typedef struct s_launcherlayout
{
	t_rect	panel;
	t_rect	search;
	t_rect	title;
	t_rect	tile[LUNA_LAUNCHER_TILES];
	t_rect	sep;
	t_rect	avatar;
	t_rect	user;
	t_rect	logoff;
	t_rect	poweroff;
	int32_t	ntiles;
}	t_launcherlayout;

void	luna_launcher_layout(t_rect panel, int32_t ntiles,
			t_launcherlayout *out);
int32_t	luna_launcher_hit(const t_launcherlayout *l, int32_t x, int32_t y);
t_rect	luna_launcher_place(int32_t screen_h, int32_t bar_h);
t_rect	luna_launcher_place_in(int32_t screen_w, int32_t screen_h,
			int32_t bar_h);
void	luna_launcher(t_surface *s, const t_launcher *l, t_rect panel);

# define LUNA_LM_QUERY_MAX 48
# define LUNA_LM_LABEL_MAX 48
# define LUNA_LM_APPS_MAX 80
# define LUNA_LM_RECENTS 4
# define LUNA_LM_ROWS 4
# define LUNA_LM_NONE -1

typedef enum e_lmview
{
	LUNA_LV_PINNED = 0,
	LUNA_LV_RECENT,
	LUNA_LV_ALL,
	LUNA_LV_SEARCH,
	LUNA_LV_COUNT
}	t_lmview;

typedef struct s_lmapp
{
	char		label[LUNA_LM_LABEL_MAX];
	uint32_t	id;
	t_iconid	icon;
}	t_lmapp;

typedef struct s_lmodel
{
	char		query[LUNA_LM_QUERY_MAX];
	uint32_t	qlen;
	t_lmapp		apps[LUNA_LM_APPS_MAX];
	uint32_t	napps;
	uint32_t	recent[LUNA_LM_RECENTS];
	uint32_t	nrecent;
	uint32_t	shown[LUNA_LM_APPS_MAX];
	uint32_t	nshown;
	int32_t		tab;
	int32_t		top;
	int32_t		sel;
	int32_t		hot;
}	t_lmodel;

void	luna_lm_init(t_lmodel *m);
int		luna_lm_set_apps(t_lmodel *m, const t_lmapp *apps, uint32_t n);
int		luna_lm_type(t_lmodel *m, uint32_t codepoint);
int		luna_lm_backspace(t_lmodel *m);
void	luna_lm_clear_query(t_lmodel *m);
void	luna_lm_set_tab(t_lmodel *m, int32_t tab);
t_lmview	luna_lm_view(const t_lmodel *m);
void	luna_lm_touch(t_lmodel *m, uint32_t id);
int32_t	luna_lm_move(t_lmodel *m, int32_t delta);
int32_t	luna_lm_select(t_lmodel *m, int32_t row);
int32_t	luna_lm_scroll(t_lmodel *m, int32_t delta);
int		luna_lm_selected(const t_lmodel *m, uint32_t *id);
int32_t	luna_lm_match(const char *label, const char *query);
const t_lmapp	*luna_lm_row(const t_lmodel *m, int32_t row);

t_rect	luna_launcher_tab_rect(const t_launcherlayout *l, int32_t tab);
t_rect	luna_launcher_row_rect(const t_launcherlayout *l, int32_t row);
int32_t	luna_launcher_tab_hit(const t_launcherlayout *l, int32_t x, int32_t y);
int32_t	luna_launcher_row_hit(const t_launcherlayout *l, int32_t x, int32_t y);
void	luna_launcher_view(t_surface *s, const t_launcher *l,
			const t_lmodel *m, t_rect panel);

#endif
