#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "velum/abi/abi_input.h"
#include "render.h"

void	render_shot(const char *name)
{
	t_surface	screen;
	char		path[128];

	mkdir(A20_RENDER, 0755);
	gfx_surface_init(&screen, calloc(SCREEN_W * SCREEN_H, 4), SCREEN_W,
		SCREEN_H);
	fwm_compose(&screen);
	snprintf(path, sizeof(path), A20_RENDER "/%s.ppm", name);
	ppm_write(&screen, path);
	free(screen.px);
}

void	drv_move(t_shell *sh, uint32_t win, t_point p)
{
	t_uimsg	m;

	msg_mouse(&m, win, INP_MOUSE_MOVE, p);
	sh_dispatch(sh, &m);
}

void	drv_click(t_shell *sh, uint32_t win, t_point p)
{
	t_uimsg	m;

	msg_button(&m, win, INP_MOUSE_DOWN, p);
	sh_dispatch(sh, &m);
	msg_button(&m, win, INP_MOUSE_UP, p);
	sh_dispatch(sh, &m);
}

void	drv_key(t_shell *sh, uint32_t win, uint32_t code)
{
	t_uimsg	m;

	msg_key(&m, win, INP_KEY_DOWN, code);
	sh_dispatch(sh, &m);
	msg_key(&m, win, INP_KEY_UP, code);
	sh_dispatch(sh, &m);
}

void	drv_type(t_shell *sh, uint32_t win, const char *text)
{
	t_uimsg	m;

	while (*text)
	{
		msg_key(&m, win, INP_CHAR, (uint8_t)(*text));
		sh_dispatch(sh, &m);
		text++;
	}
}
