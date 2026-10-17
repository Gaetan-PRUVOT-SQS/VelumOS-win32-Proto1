#ifndef LT_CTL_GAL_H
# define LT_CTL_GAL_H

# include "lt_gal.h"

void	lt_ctl_buttons(t_surface *s, int32_t x, int32_t y, bool primary);
void	lt_ctl_choices(t_surface *s, int32_t x, int32_t y);
void	lt_ctl_fields(t_surface *s, int32_t x, int32_t y);
void	lt_ctl_bars(t_surface *s, int32_t x, int32_t y);
void	lt_ctl_menu(t_surface *s, int32_t x, int32_t y);

#endif
