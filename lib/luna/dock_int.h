#ifndef DOCK_INT_H
# define DOCK_INT_H

# include "luna_int.h"
# include "velum/luna_dock.h"
# include "velum/theme.h"

typedef struct s_ldplate
{
	t_rect		r;
	int32_t		rad;
	t_color		c;
	int32_t		thick;
}	t_ldplate;

typedef int32_t	(*t_ldcov)(const t_ldplate *p, int32_t x, int32_t y);

int32_t	ld_cov(const t_ldplate *p, int32_t x, int32_t y);
void	ld_paint(t_surface *s, const t_ldplate *p, t_ldcov fn);
void	ld_plate(t_surface *s, const t_ldplate *p);
void	ld_ring(t_surface *s, const t_ldplate *p);
void	ld_rim(t_surface *s, const t_ldplate *p);
bool	ld_dark(void);
t_color	ld_pick(t_color light, t_color dark);
t_color	ld_accent(void);

#endif
