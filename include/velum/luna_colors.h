#ifndef LUNA_COLORS_H
# define LUNA_COLORS_H

# include "velum/luna.h"

typedef enum e_lunacolor
{
	LUNA_COLOR_TEXT = 0,
	LUNA_COLOR_TEXT_2,
	LUNA_COLOR_MUTED,
	LUNA_COLOR_DIM,
	LUNA_COLOR_ACCENT,
	LUNA_COLOR_ACCENT_HI,
	LUNA_COLOR_ACCENT_TEXT,
	LUNA_COLOR_DANGER,
	LUNA_COLOR_WINDOW,
	LUNA_COLOR_PANEL,
	LUNA_COLOR_SELECTION_BG,
	LUNA_COLOR_SELECTION_RING,
	LUNA_COLORS
}	t_lunacolor;

t_color	luna_color(t_lunacolor id);
t_color	luna_color_text_2(void);
t_color	luna_color_muted(void);
t_color	luna_color_dim(void);
t_color	luna_color_danger(void);
t_color	luna_color_accent_hi(void);
t_color	luna_color_accent_text(void);
t_color	luna_color_panel(void);
t_color	luna_color_selection_bg(void);
t_color	luna_color_selection_ring(void);

#endif
