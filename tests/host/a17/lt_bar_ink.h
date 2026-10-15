#ifndef LT_BAR_INK_H
# define LT_BAR_INK_H

# include "lt_contrast.h"
# include "velum/luna_dock.h"
# include "velum/theme.h"

# define LKB_W 1280
# define LKB_H 800
# define LKB_BAR_H 80
# define LKB_TASK_W 160
# define LKB_SCR 16384

typedef struct s_lkbar
{
	t_lt			*t;
	t_taskbarlayout	lay;
	bool			dark;
}	t_lkbar;

t_color	lkb_ink(bool dark);
void	lkb_begin(t_lkbar *b, t_lt *t, bool dark);
void	lkb_task(const t_lkbar *b, int32_t i, t_lunastate st);
void	lkb_clock(const t_lkbar *b);

#endif
