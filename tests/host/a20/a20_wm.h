#ifndef A20_WM_H
# define A20_WM_H

# include <stdint.h>
# include "velum/libk.h"
# include "velum/wm.h"

static inline void	mk_info(t_wminfo *w, uint32_t id, uint32_t style,
		uint32_t state)
{
	memset(w, 0, sizeof(*w));
	w->h.magic = WM_MAGIC;
	w->h.type = WMS_WIN_ADD;
	w->h.size = sizeof(*w);
	w->h.window = id;
	w->style = style;
	w->state = state;
	strlcpy(w->title, "Fenetre", sizeof(w->title));
}

#endif
