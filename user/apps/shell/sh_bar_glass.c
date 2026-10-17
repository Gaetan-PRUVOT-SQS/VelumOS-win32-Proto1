#include <stdlib.h>
#include "velum/err.h"
#include "velum/luna_launcher.h"
#include "shell.h"

static size_t	launcher_px(void)
{
	size_t	panel;
	size_t	maquette;

	panel = luna_glass_need(rect_make(0, 0, LUNA_LAUNCHER_WIDTH,
				LUNA_LAUNCHER_HEIGHT));
	maquette = luna_glass_need(rect_make(0, 0, SH_GLASS_LAUNCHER_W,
				SH_GLASS_LAUNCHER_H));
	if (panel > maquette)
		return (panel);
	return (maquette);
}

size_t	sh_glass_px(int32_t screen_w)
{
	size_t	dock;
	size_t	launcher;

	dock = luna_glass_need(rect_make(0, 0, screen_w - 2 * VT_DOCK_MARGIN,
				VT_DOCK_H));
	launcher = launcher_px();
	if (dock > launcher)
		return (dock);
	return (launcher);
}

int	sh_glass_alloc(t_shell *sh, size_t px)
{
	uint32_t	*buf;

	sh_glass_free(sh);
	if (px == 0 || px > SIZE_MAX / sizeof(uint32_t))
		return (E_INVAL);
	buf = malloc(px * sizeof(uint32_t));
	if (!buf)
		return (E_NOMEM);
	sh->glass_px = buf;
	sh->glass_n = px;
	luna_glass_scratch(buf, px);
	return (0);
}

int	sh_glass_make(t_shell *sh)
{
	return (sh_glass_alloc(sh, sh_glass_px((int32_t)sh->info.screen_w)));
}

void	sh_glass_free(t_shell *sh)
{
	luna_glass_scratch(NULL, 0);
	free(sh->glass_px);
	sh->glass_px = NULL;
	sh->glass_n = 0;
}
