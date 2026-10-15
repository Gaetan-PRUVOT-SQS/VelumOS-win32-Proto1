#include "mouse.h"
#include "velum/err.h"

int	accel_set(t_accel *a, uint32_t speed, uint32_t enabled)
{
	if (speed < INPUT_MOUSE_SPEED_MIN || speed > INPUT_MOUSE_SPEED_MAX)
		return (E_INVAL);
	if (enabled > 1)
		return (E_INVAL);
	a->speed = speed;
	a->enabled = enabled;
	a->rem_x = 0;
	a->rem_y = 0;
	return (0);
}

void	mouse_init(t_mouse *m)
{
	accel_init(&m->accel);
	m->buttons = 0;
}
