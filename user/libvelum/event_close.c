#include "velum/vobj.h"
#include "velum/vsync.h"

void	v_event_destroy(t_vevent *event)
{
	if (!event)
		return ;
	if (event->handle)
		v_close(event->handle);
	event->handle = 0;
	event->manual = 0;
}
