#ifndef LUNA_CTRL_H
# define LUNA_CTRL_H

# include "luna_int.h"

# define LCT_RAD_CTRL 12
# define LCT_RAD_FIELD 10
# define LCT_RAD_GROUP 14
# define LCT_RAD_ITEM 8
# define LCT_RAD_MENU 10
# define LCT_RIM_PRIMARY 0x73ffffff
# define LCT_FOCUS_GAP 2
# define LCT_FOCUS_W 2

typedef struct s_lrfill
{
	t_rect	r;
	int32_t	rad;
	t_color	top;
	t_color	bottom;
}	t_lrfill;

typedef struct s_lchoice
{
	t_rect		r;
	int32_t		rad;
	t_lunastate	st;
	bool		on;
	bool		solid;
}	t_lchoice;

t_lrfill	lp_rmake(t_rect r, int32_t rad, t_color top, t_color bottom);
int32_t		lp_rcov(const t_lrfill *f, t_point p);
void		lp_rfill(t_surface *s, const t_lrfill *f);
void		lp_rring(t_surface *s, const t_lrfill *f, int32_t w);
void		lp_rtop(t_surface *s, const t_lrfill *f);
void		lp_rfocus(t_surface *s, t_rect r, int32_t rad, t_color c);
void		lp_choice(t_surface *s, const t_lchoice *c);
bool		lp_btn_primary(const t_lunabtn *b);
bool		lp_dark(void);
t_color		lp_tone_alpha(t_color c, uint32_t a);
t_color		lp_tone_hover(void);
t_color		lp_tone_press(void);
t_color		lp_tone_track(void);

#endif
