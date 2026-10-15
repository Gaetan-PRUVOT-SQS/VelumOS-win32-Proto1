#include "inp_queue.h"
#include "velum/klog.h"

static const char *const	g_type_names[] = {"none", "key_down", "key_up",
	"char", "mouse_move", "mouse_down", "mouse_up", "wheel"};

void	inplog_event(const t_inpevent *ev)
{
	const char	*name;

	name = "unknown";
	if (ev->type < sizeof(g_type_names) / sizeof(g_type_names[0]))
		name = g_type_names[ev->type];
	kprintf("inp %s code=0x%x sc=0x%x mods=0x%x x=%d y=%d\n", name,
		ev->code, ev->scancode, ev->mods, (int)ev->x, (int)ev->y);
}
