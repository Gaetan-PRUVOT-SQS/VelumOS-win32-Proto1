#ifndef LT_GEO_H
# define LT_GEO_H

# include <stdint.h>
# include "luna_int.h"

# define LT_TOOL 0x0043
# define LT_MINONLY 0x0007
# define LT_CLOSEONLY 0x0003
# define LT_DESKTOP 0x0201
# define LT_APPBAR 0x0401

typedef struct s_ltcase
{
	const char	*name;
	t_rect		outer;
	uint32_t	style;
	bool		maximized;
	t_rect		client;
	t_rect		caption;
	t_rect		sysmenu;
	t_rect		btn[3];
}	t_ltcase;

extern const t_ltcase	g_geo_cases[];
extern const int		g_geo_count;

t_lunawin	lt_win(t_rect outer, uint32_t style, bool maximized);
bool		lt_rect_eq(t_rect a, t_rect b);
bool		lt_rect_in(t_rect a, t_rect b);
bool		lt_rect_disjoint(t_rect a, t_rect b);
const char	*lt_rect_str(t_rect r);
bool		lt_geo_check(const t_lunawin *w);

#endif
