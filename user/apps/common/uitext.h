#ifndef UITEXT_H
# define UITEXT_H

# include <stdint.h>
# include "velum/font.h"
# include "velum/gfx.h"
# include "uicolor.h"

# define UI_WRAP_MAX 128

typedef struct s_uitext
{
	t_fontid	font;
	t_point		at;
	t_color		color;
	int32_t		max_w;
}	t_uitext;

void	ui_text(t_surface *s, const t_uitext *st, const char *text);
void	ui_text_shadow(t_surface *s, const t_uitext *st, const char *text);
void	ui_text_wrap(t_surface *s, const t_uitext *st, const char *text);
int32_t	ui_text_width(t_fontid font, const char *text);
int32_t	ui_font_height(t_fontid font);

#endif
