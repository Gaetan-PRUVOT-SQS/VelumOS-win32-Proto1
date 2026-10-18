#ifndef SH_DESK_H
# define SH_DESK_H

# include <stdint.h>
# include "velum/gfx.h"
# include "velum/luna.h"

# define SH_ICON_TOP 0
# define SH_ICON_BIAS_X 1
# define SH_LABEL_GAP 7
# define SH_LABEL_FONT FONT_INS_12
# define SH_LABEL_TEXT_TOP 1
# define SH_LABEL_PAD_X 8
# define SH_LABEL_PAD_Y 2
# define SH_LABEL_RADIUS 8
# define SH_PLATE_LIGHT 0x80ffffffu
# define SH_PLATE_DARK 0x80141824u
# define SH_SEL_RADIUS 12
# define SH_SEL_FILL_A 31
# define SH_SEL_RING_A 115
# define SH_SEL_RING_A_DARK 128
# define SH_SHADOW_BLUR 4
# define SH_SHADOW_PASSES 3
# define SH_SHADOW_PAD 16
# define SH_SHADOW_DY 4
# define SH_SHADOW_A 64
# define SH_SHADOW_RGB 0x28285au
# define SH_SHADOW_MAX 256
# define SH_DLGW_RUN 380
# define SH_DLGH_RUN 124
# define SH_DLGW_POWER 360
# define SH_DLGH_POWER 108
# define SH_DLGW_NOTE 340
# define SH_DLGH_NOTE 108

typedef struct s_selpaint
{
	t_surface	*s;
	t_rect		cell;
	uint32_t	rgb;
	uint32_t	ring_a;
}	t_selpaint;

typedef struct s_shadowjob
{
	uint32_t	*px;
	uint8_t		*a;
	uint8_t		*tmp;
	int32_t		n;
	int32_t		size;
	t_iconid	id;
}	t_shadowjob;

int32_t	sh_sel_depth(t_rect cell, int32_t x, int32_t y);
void	sh_sel_paint(const t_selpaint *k);
void	sh_blur_alpha(uint8_t *a, uint8_t *tmp, int32_t n);
void	sh_icon_shadow(t_surface *s, t_point p, t_iconid id, int32_t size);

#endif
