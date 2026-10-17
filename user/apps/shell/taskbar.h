#ifndef TASKBAR_H
# define TASKBAR_H

# include <stdint.h>
# include "velum/gfx.h"
# include "velum/theme.h"
# include "tasklist.h"

# define TB_BTN_MAX_W 160
# define TB_BTN_MIN_W 40
# define TB_GAP 6
# define TB_STRIP_PAD_L 4
# define TB_SIDE_GAP_MAX 64
# define TB_DIM_MAX 65536U

typedef struct s_tbmetrics
{
	int32_t	w;
	int32_t	h;
	int32_t	margin;
	int32_t	pad;
	int32_t	gap;
	int32_t	start_d;
	int32_t	tray_w;
	int32_t	tray_h;
	int32_t	task_h;
}	t_tbmetrics;

typedef struct s_tbregions
{
	t_rect	start;
	t_rect	strip;
	t_rect	tray;
}	t_tbregions;

int		tb_regions(const t_tbmetrics *m, t_tbregions *out);
int		tb_layout(t_rect strip, uint32_t count, t_rect *out);
int32_t	tb_hit(const t_rect *r, uint32_t n, int32_t x, int32_t y);

#endif
