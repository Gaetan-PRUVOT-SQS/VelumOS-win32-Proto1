#ifndef LAUNCHER_INT_H
# define LAUNCHER_INT_H

# include "luna_int.h"
# include "velum/luna_launcher.h"

# define LL_PAD 18
# define LL_GAP 6
# define LL_SEARCH_H 48
# define LL_SEARCH_ICON 26
# define LL_SEARCH_PAD 10
# define LL_SEARCH_PAD_R 18
# define LL_TITLE_H 16
# define LL_TITLE_GAP 10
# define LL_TITLE_PAD 6
# define LL_TILE_H 96
# define LL_TILE_TOP 14
# define LL_FOOT_H 36
# define LL_FOOT_PAD 10
# define LL_FOOT_GAP 11
# define LL_AVATAR 34
# define LL_LOGOFF_W 135
# define LL_ICON 44
# define LL_POWER_ICON 26
# define LL_LABEL_H 16
# define LL_FOCUS_GAP 2
# define LL_MIN_W 320
# define LL_MIN_H 289
# define LL_HOT_L 0x9effffff
# define LL_HOT_D 0x1affffff
# define LL_HOT_RIM_L 0xffffffff
# define LL_HOT_RIM_D 0x26ffffff
# define LL_BTN_RIM_L 0xffffffff
# define LL_BTN_RIM_D 0x1fffffff
# define LL_SEP_L 0x99ffffff
# define LL_SEP_D 0x14ffffff
# define LL_INPUT_RIM_L 0x14141824
# define LL_INPUT_RIM_D 0x1affffff
# define LL_ST_HOT 1
# define LL_ST_FOCUS 2
# define LL_ROW_H 26
# define LL_ROW_ICON 20
# define LL_ROW_PAD 8
# define LL_TAB_PAD 4
# define LL_BAR_W 3
# define LL_BAR_MIN 12

void	lp_launch_fill(t_surface *s, t_rect r, int32_t rad, t_color c);
t_color	lp_launch_tone(t_color light, t_color dark);
void	lp_launch_ring(t_surface *s, t_rect r, int32_t rad, t_color c);
void	lp_launch_focus(t_surface *s, t_rect r, int32_t rad);
void	lp_launch_tile(t_surface *s, t_rect r, const t_launchertile *t,
			int32_t st);
void	lp_launch_search(t_surface *s, const t_launcherlayout *l);
void	lp_launch_title(t_surface *s, const t_launcherlayout *l);
void	lp_launch_foot(t_surface *s, const t_launcher *l,
			const t_launcherlayout *lay);
int32_t	lp_launch_state(const t_launcher *l, int32_t id);
void	lp_launch_tiles(t_surface *s, const t_launcher *l,
			const t_launcherlayout *lay);
void	lp_launch_query(t_surface *s, const t_launcherlayout *l,
			const char *query);
void	lp_launch_tabs(t_surface *s, const t_launcherlayout *l, int32_t active);
void	lp_launch_rows(t_surface *s, const t_launcherlayout *l,
			const t_lmodel *m);
uint32_t	lp_lm_fold(uint32_t *out, uint32_t max, const char *s);
int32_t	lp_lm_cmp(const char *a, const char *b);
void	lp_lm_refresh(t_lmodel *m);

#endif
