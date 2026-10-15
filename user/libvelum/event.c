#include "velum/err.h"
#include "velum/vobj.h"
#include "velum/vsync.h"

int	v_event_init(t_vevent *event, int manual, int initial)
{
	int64_t	h;

	if (!event)
		return (E_FAULT);
	h = v_event_create(manual, initial);
	if (h < 0)
		return ((int)h);
	event->handle = (uint32_t)h;
	event->manual = manual != 0;
	return (0);
}

int	v_event_set(t_vevent *event)
{
	if (!event || !event->handle)
		return (E_INVAL);
	return (v_event_op(event->handle, EV_SET));
}

int	v_event_reset(t_vevent *event)
{
	if (!event || !event->handle)
		return (E_INVAL);
	return (v_event_op(event->handle, EV_RESET));
}

int	v_event_pulse(t_vevent *event)
{
	if (!event || !event->handle)
		return (E_INVAL);
	return (v_event_op(event->handle, EV_PULSE));
}

int	v_event_wait(t_vevent *event, uint64_t timeout_ns)
{
	if (!event || !event->handle)
		return (E_INVAL);
	return (v_wait(event->handle, timeout_ns));
}
