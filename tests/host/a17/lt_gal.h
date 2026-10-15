#ifndef LT_GAL_H
# define LT_GAL_H

# include "lt.h"
# include "lt_geo.h"

void		lt_gal_text(t_surface *s, t_rect box, const char *txt, t_color c);
void		lt_gal_windows(t_surface *s);
void		lt_gal_buttons(t_surface *s, int32_t x, int32_t y);
void		lt_gal_checks(t_surface *s, int32_t x, int32_t y);
void		lt_gal_scrolls(t_surface *s, int32_t x, int32_t y);
void		lt_gal_misc(t_surface *s, int32_t x, int32_t y);
void		lt_gal_icons(t_surface *s, int32_t x, int32_t y);
void		lt_gal_desktop(t_surface *s, t_rect r);
t_surface	lt_crop(const t_surface *s, t_rect r);
void		lt_zoom(const t_surface *s, t_rect r, int32_t zoom, const char *p);

#endif
