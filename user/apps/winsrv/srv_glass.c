#include "velum/luna_launcher.h"
#include "ws_srv.h"

static uint64_t	glass_bytes(int32_t width)
{
	uint64_t	bar;
	uint64_t	panel;

	bar = (uint64_t)luna_glass_need(rect_make(0, 0, width, 80)) * 4;
	if (width < LUNA_LAUNCHER_WIDTH)
		return (bar);
	panel = (uint64_t)luna_glass_need(rect_make(0, 0, LUNA_LAUNCHER_WIDTH,
				LUNA_LAUNCHER_HEIGHT)) * 4;
	if (panel > bar)
		return (panel);
	return (bar);
}

void	ws_glass_open(t_wsrv *s)
{
	uint64_t	bytes;

	s->glass = NULL;
	s->glass_bytes = 0;
	luna_glass_scratch(NULL, 0);
	bytes = glass_bytes(s->back.w);
	if (bytes == 0)
		return ;
	s->glass = ws_sys_alloc(bytes);
	if (s->glass == NULL)
	{
		ws_sys_log("winsrv: verre opaque, memoire refusee");
		return ;
	}
	s->glass_bytes = bytes;
	luna_glass_scratch(s->glass, (size_t)(bytes / 4));
}

void	ws_glass_close(t_wsrv *s)
{
	luna_glass_scratch(NULL, 0);
	if (s->glass != NULL)
		ws_sys_free(s->glass, s->glass_bytes);
	s->glass = NULL;
	s->glass_bytes = 0;
}
