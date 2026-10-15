#ifndef LT_CONTRAST_H
# define LT_CONTRAST_H

# include "lt.h"
# include "fake_font.h"
# include "luna_int.h"

# define LK_AA 4.5
# define LK_SLACK 2
# define LK_UI 11

double	lk_ratio(t_color a, t_color b);
double	lk_worst(const t_surface *s, t_rect r, t_color text);
t_rect	lk_band(t_rect r, int32_t line);
void	lk_begin(t_lt *t, t_color back);
double	lk_spied(const t_surface *s);
double	lk_title(t_lt *t, uint32_t style, bool active);
void	lk_check(const char *what, double ratio);

#endif
