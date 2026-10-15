#ifndef LT_BBOX_H
# define LT_BBOX_H

# include "lt.h"
# include "luna_int.h"

t_rect	ltb_bbox(const t_surface *a, const t_surface *b, t_color bg);
bool	ltb_inside(t_rect in, t_rect out);

#endif
